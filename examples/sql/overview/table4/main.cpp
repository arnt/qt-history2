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


StatusPicker::StatusPicker( QWidget *parent, const char *name )
    : QComboBox( parent, name )
{
    QSqlCursor cur( "status" );
    cur.select( cur.index( "name" ) );

    int i = 0;
    while ( cur.next() ) {
	insertItem( cur.value( "name" ).toString(), i );
	index2id[i] = cur.value( "id" ).toInt();
	i++;
    }
}


int StatusPicker::statusId() const
{
    return index2id[ currentItem() ];
}


void StatusPicker::setStatusId( int statusid )
{
    QMap<int,int>::Iterator it;
    for ( it = index2id.begin(); it != index2id.end(); ++it ) {
	if ( it.data() == statusid ) {
	    setCurrentItem( it.key() );
	    break;
	}
    }
}


void CustomTable::paintField( QPainter * p, const QSqlField* field,
			    const QRect & cr, bool b)
{
    if ( !field )
	return;
    if ( field->name() == "statusid" ) {
	QSqlQuery query( "SELECT name FROM status WHERE id=" +
		     field->value().toString() );
	QString text;
	if ( query.next() ) {
	    text = query.value( 0 ).toString();
	}
	p->drawText( 2,2, cr.width()-4, cr.height()-4, fieldAlignment( field ), text );
    }
    else {
	QDataTable::paintField( p, field, cr, b) ;
    }
}


QWidget *CustomSqlEditorFactory::createEditor(
    QWidget *parent, const QSqlField *field )
{
    if ( field->name() == "statusid" ) {
	QWidget *editor = new StatusPicker( parent );
	return editor;
    }

    return QSqlEditorFactory::createEditor( parent, field );
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( createConnections() ) {
	QSqlCursor staffCursor( "staff" );

	CustomTable		*staffTable	= new CustomTable( &staffCursor );
	QSqlPropertyMap		*propMap	= new QSqlPropertyMap();
	CustomSqlEditorFactory	*editorFactory	= new CustomSqlEditorFactory();
	propMap->insert( "StatusPicker", "statusid" );
	staffTable->installPropertyMap( propMap );
	staffTable->installEditorFactory( editorFactory );

	app.setMainWidget( staffTable );

	staffTable->addColumn( "forename", "Forename" );
	staffTable->addColumn( "surname",  "Surname" );
	staffTable->addColumn( "salary",   "Annual Salary" );
	staffTable->addColumn( "statusid", "Status" );

	QStringList order = QStringList() << "surname" << "forename";
	staffTable->setSort( order );

	staffTable->refresh();
	staffTable->show();

	return app.exec();
    }

    return 1;
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
