/****************************************************************************
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qfile.h>

#define DRIVER       "QMYSQL3" 	/* see the Qt SQL documentation for a list of available drivers */
#define DATABASE     "testdb"  	/* the name of your database */
#define USER         "troll"  	/* user name with appropriate rights */
#define PASSWORD     "trond"	/* password for USER */
#define HOST         "horsehead.troll.no" /* host on which the database is running */

int main( int argc, char ** argv )
{

    QApplication a( argc, argv, FALSE );
    QSqlDatabase * db = QSqlDatabase::addDatabase( DRIVER );
    db->setDatabaseName( DATABASE );
    db->setUserName( USER );
    db->setPassword( PASSWORD );
    db->setHostName( HOST );
    if ( !db->open() ) {
	qWarning( db->lastError().databaseText() );
	return 1;
    }

    if ( argc < 2 ) {
	qWarning( "Usage: %s <filename>", argv[0] );
	return 1;
    }
    
    // read a file which we want to insert into the database
    QFile f( argv[1] );
    if ( !f.open( IO_ReadOnly ) ) {
	qWarning( "Unable to open data file '%s' - exiting", argv[1] );
	return 1;
    }
    QByteArray binaryData( f.size() );
    f.readBlock( binaryData.data(), f.size() );
    qWarning( "Data size: %d", binaryData.size() );
    
    // create a table with a binary field
    QSqlQuery q;
    if ( !q.exec( "CREATE TABLE blobexample ( id INT PRIMARY KEY, binfield LONGBLOB )" ) ) {
	qWarning( "Unable to create table - exiting" );
	return 1;
    }

    // insert a BLOB into the table
    if ( !q.prepare( "INSERT INTO blobexample ( id, binfield ) VALUES ( ?, ? )" ) ) {
	qWarning( "Unable to prepare query - exiting" );
	return 1;
    }
    q.bindValue( 0, 1 );
    q.bindValue( 1, binaryData );
    if ( !q.exec() ) {
	qWarning( "Unable to execute prepared query - exiting" );
    }

    // read the BLOB back from the database
    if ( !q.exec( "SELECT id, binfield FROM blobexample" ) ) {
	qWarning( "Unable to execute query - exiting" );
    }
    while ( q.next() ) {
	qWarning( "BLOB id: %d", q.value( 0 ).toInt() );
	qWarning( "BLOB size: %d", q.value( 1 ).toByteArray().size() );
    }

    if ( !q.exec( "DROP TABLE blobexample" ) ) {
	qWarning( "Unable to drop table - exiting" );
	return 1;
    }
    return 0;
}
