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
#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqlform.h>
#include "../login.h"

bool createConnections();


class FormDialog : public QDialog
{
    public:
	FormDialog();
};


FormDialog::FormDialog()
{
    QLabel *forenameLabel   = new QLabel( "Forename:", this );
    QLabel *forenameDisplay = new QLabel( this );
    QLabel *surnameLabel    = new QLabel( "Surname:", this );
    QLabel *surnameDisplay  = new QLabel( this );
    QLabel *salaryLabel	    = new QLabel( "Salary:", this );
    QLineEdit *salaryEdit   = new QLineEdit( this );

    QGridLayout *grid = new QGridLayout( this );
    grid->addWidget( forenameLabel,	0, 0 );
    grid->addWidget( forenameDisplay,	0, 1 );
    grid->addWidget( surnameLabel,	1, 0 );
    grid->addWidget( surnameDisplay,	1, 1 );
    grid->addWidget( salaryLabel,	2, 0 );
    grid->addWidget( salaryEdit,	2, 1 );
    grid->activate();

    QSqlCursor staffCursor( "staff" );
    staffCursor.select();
    staffCursor.next();

    QSqlForm sqlForm( this );
    sqlForm.setRecord( staffCursor.primeUpdate() );
    sqlForm.insert( forenameDisplay, "forename" );
    sqlForm.insert( surnameDisplay, "surname" );
    sqlForm.insert( salaryEdit, "salary" );
    sqlForm.readFields();
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( ! createConnections() ) return 1;

    FormDialog *formDialog = new FormDialog();
    formDialog->show();
    app.setMainWidget( formDialog );

    return app.exec();
}


bool createConnections()
{
    // create the default database connection
    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( DB_SALES_DRIVER );
    if ( ! defaultDB ) {
	qWarning( "Failed to connect to driver" );
	return FALSE;
    }
    defaultDB->setDatabaseName( DB_SALES_DBNAME );
    defaultDB->setUserName( DB_SALES_USER );
    defaultDB->setPassword( DB_SALES_PASSWD );
    defaultDB->setHostName( DB_SALES_HOST );
    if ( ! defaultDB->open() ) { 
	qWarning( "Failed to open sales database: " + 
		  defaultDB->lastError().driverText() );
	qWarning( defaultDB->lastError().databaseText() );
	return FALSE;
    }

    // create a named connection to oracle
    QSqlDatabase *oracle = QSqlDatabase::addDatabase( DB_ORDERS_DRIVER, "ORACLE" );
    if ( ! oracle ) {
	qWarning( "Failed to connect to oracle driver" );
	return FALSE;
    }
    oracle->setDatabaseName( DB_ORDERS_DBNAME );
    oracle->setUserName( DB_ORDERS_USER );
    oracle->setPassword( DB_ORDERS_PASSWD );
    oracle->setHostName( DB_ORDERS_HOST );
    if ( ! oracle->open() ) {
	qWarning( "Failed to open orders database: " + 
		  oracle->lastError().driverText() );
	qWarning( oracle->lastError().databaseText() );
	return FALSE;
    }

    return TRUE;
}


