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
    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( "QODBC" );

    defaultDB->setDatabaseName( "sales" );
    defaultDB->setUserName( "salesuser" );
    defaultDB->setPassword( "salespw" );
    defaultDB->setHostName( "saleshost" );

    if ( defaultDB->open() ) {
        // Database successfully opened; we can now issue SQL commands.
    }

    return 0;
}

