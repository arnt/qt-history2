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
#include <qsqlcursor.h>

bool create_connections();

int main( int argc, char *argv[] )
{
   if ( create_connections() ) {
        QSqlCursor cur( "staff" ); 
        QStringList fields = QStringList() << "surname" << "forename";
        QSqlIndex order = cur.index( fields );
        cur.select( order ); 
        while ( cur.next() ) {
	    debug( cur.value( "id" ).toString() + ": " +
		   cur.value( "surname" ).toString() + " " +
		   cur.value( "forename" ).toString() );
        }
    }

    return 0;
}


bool create_connections()
{

    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( "QPSQL6" );
    defaultDB->setDatabaseName( "testdb" );
    defaultDB->setUserName( "db" );
    defaultDB->setPassword( "db" );
    defaultDB->setHostName( "silverfish" );
    if ( ! defaultDB->open() ) { 
	qWarning( "Failed to open sales database: " + 
		  defaultDB->lastError().driverText() );
	qWarning( defaultDB->lastError().databaseText() );
	return false;
    }

    QSqlDatabase *oracle = QSqlDatabase::addDatabase( "QPSQL6", "ORACLE" );
    oracle->setDatabaseName( "pingpong" );
    oracle->setUserName( "db" );
    oracle->setPassword( "db" );
    oracle->setHostName( "silverfish" );
    if ( ! oracle->open() ) {
	qWarning( "Failed to open orders database: " + 
		  oracle->lastError().driverText() );
	qWarning( oracle->lastError().databaseText() );
	return false;
    }

    return true;
}



