/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "main.h"

FormDialog::FormDialog()
    : staffCursor( "staff" )
{
    staffCursor.setTrimmed( "forename", TRUE );
    staffCursor.setTrimmed( "surname",  TRUE );

    QLabel	*forenameLabel = new QLabel( "Forename:", this );
    QLineEdit	*forenameEdit  = new QLineEdit( this );
    QLabel	*surnameLabel  = new QLabel( "Surname:", this );
    QLineEdit	*surnameEdit   = new QLineEdit( this );
    QLabel	*salaryLabel   = new QLabel( "Salary:", this );
    QLineEdit	*salaryEdit    = new QLineEdit( this );
    QPushButton *saveButton    = new QPushButton( "&Save", this );
    connect( saveButton, SIGNAL(clicked()), this, SLOT(save()) );

    QGridLayout *grid = new QGridLayout( this );
    grid->addWidget( forenameLabel, 0, 0 );
    grid->addWidget( forenameEdit,  0, 1 );
    grid->addWidget( surnameLabel,  1, 0 );
    grid->addWidget( surnameEdit,   1, 1 );
    grid->addWidget( salaryLabel,   2, 0 );
    grid->addWidget( salaryEdit,    2, 1 );
    grid->addWidget( saveButton,    3, 0 );
    grid->activate();

    idIndex = staffCursor.index( "id" );
    staffCursor.select( idIndex );
    staffCursor.first();

    sqlForm = new QSqlForm( this );
    sqlForm->setRecord( staffCursor.primeUpdate() );
    sqlForm->insert( forenameEdit, "forename" );
    sqlForm->insert( surnameEdit, "surname" );
    sqlForm->insert( salaryEdit, "salary" );
    sqlForm->readFields();
}


FormDialog::~FormDialog()
{

}


void FormDialog::save()
{
    sqlForm->writeFields();
    staffCursor.update();
    staffCursor.select( idIndex );
    staffCursor.first();
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( ! createConnections() )
	return 1;

    FormDialog *formDialog = new FormDialog();
    formDialog->show();
    app.setMainWidget( formDialog );

    return app.exec();
}


bool createConnections()
{

    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( DB_SALES_DRIVER );
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

    QSqlDatabase *oracle = QSqlDatabase::addDatabase( DB_ORDERS_DRIVER, "ORACLE" );
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
