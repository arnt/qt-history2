/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "dbconnectionsimpl.h"
#include <qlist.h>
#include "project.h"
#include <qlistbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsqldatabase.h>

/*
 *  Constructs a DatabaseConnection which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DatabaseConnection::DatabaseConnection( Project *pro, QWidget* parent,  const char* name, bool modal, WFlags fl )
    : DatabaseConnectionBase( parent, name, modal, fl ), project( pro )
{
    QList<Project::DatabaseConnection> lst = project->databaseConnections();
    for ( Project::DatabaseConnection *conn = lst.first(); conn; conn = lst.next() )
	listConnections->insertItem( conn->name );
    comboDriver->insertStringList( QSqlDatabase::drivers() );
    enableAll( FALSE );
}

DatabaseConnection::~DatabaseConnection()
{
}

void DatabaseConnection::deleteConnection()
{
}

void DatabaseConnection::newConnection()
{
    enableAll( TRUE );
    listConnections->clearSelection();
    listConnections->setCurrentItem( FALSE );
    editName->setText( "(default)" ); // #### only if we don't have already a default connection
}

void DatabaseConnection::doConnect()
{
    if ( listConnections->currentItem() == -1 ) { // new connection
	// ### do error checking for duplicated connection names
	Project::DatabaseConnection *conn = new Project::DatabaseConnection( project );
	conn->name = editName->text();
	conn->driver = comboDriver->lineEdit()->text();
	conn->dbName = comboDatabase->lineEdit()->text();
	conn->username = editUsername->text();
	conn->password = editPassword->text();
	conn->hostname = editHostname->text();
	if ( conn->refreshCatalog() ) {
	    project->addDatabaseConnection( conn );
	    listConnections->insertItem( conn->name );
	    listConnections->setCurrentItem( listConnections->count() - 1 );
	} else {
	    // ### error handling
	    delete conn;
	}
    } else { // sync // ### should this do something else? right now it just overwrites all info about the connection...
	Project::DatabaseConnection *conn = project->databaseConnection( listConnections->currentText() );
	conn->refreshCatalog();
    }
}

static bool blockChanges = FALSE;

void DatabaseConnection::currentConnectionChanged( const QString &s )
{
    Project::DatabaseConnection *conn = project->databaseConnection( s );
    blockChanges = TRUE;
    enableAll( (bool)conn );
    blockChanges = FALSE;
    if ( !conn)
	return;
    blockChanges = TRUE;
    editName->setText( conn->name );
    blockChanges = FALSE;
    comboDriver->lineEdit()->setText( conn->driver );
    comboDatabase->lineEdit()->setText( conn->dbName );
    editUsername->setText( conn->username );
    editPassword->setText( conn->password );
    editHostname->setText( conn->hostname );
}

void DatabaseConnection::connectionNameChanged( const QString &s )
{
    if ( listConnections->currentItem() == -1 || blockChanges )
	return;
    listConnections->changeItem( s, listConnections->currentItem() );
}

void DatabaseConnection::enableAll( bool b )
{
    editName->setEnabled( b );
    editName->setText( "" );
    comboDriver->setEnabled( b );
    comboDriver->lineEdit()->setText( "" );
    comboDatabase->setEnabled( b );
    comboDatabase->clear();
    editUsername->setEnabled( b );
    editUsername->setText( "" );
    editPassword->setEnabled( b );
    editPassword->setText( "" );
    editHostname->setEnabled( b );
    editHostname->setText( "" );
    buttonConnect->setEnabled( b );
}
