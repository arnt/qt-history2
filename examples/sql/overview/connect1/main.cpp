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


int main( int argc, char *argv[] )
{
    QSqlDatabase *db = QSqlDatabase::addDatabase( "QPSQL6" );

    db->setDatabaseName( "pingpong" );
    db->setUserName( "db" );
    db->setPassword( "db" );
    db->setHostName( "silverfish" );

    if ( db->open() ) {
        // Database successfully opened; we can now issue SQL commands.
    }

    return 0;
}

