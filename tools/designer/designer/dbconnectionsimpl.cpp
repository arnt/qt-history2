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
#include <qgroupbox.h>
#include <qlayout.h>
#include "project.h"
#include <qlistbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsqldatabase.h>

static bool blockChanges = FALSE;

/*
 *  Constructs a DatabaseConnectionEditor which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DatabaseConnectionEditor::DatabaseConnectionEditor( Project *pro, QWidget* parent,  const char* name, bool modal, WFlags fl )
    : DatabaseConnectionBase( parent, name, modal, fl ), project( pro )
{
    connectionWidget = new DatabaseConnectionWidget( grp );
    grpLayout->addWidget( connectionWidget, 0, 0 );
#ifndef QT_NO_SQL
    QList<DatabaseConnection> lst = project->databaseConnections();
    for ( DatabaseConnection *conn = lst.first(); conn; conn = lst.next() )
	listConnections->insertItem( conn->name() );
    connectionWidget->comboDriver->insertStringList( QSqlDatabase::drivers() );
#endif
    enableAll( FALSE );
}

DatabaseConnectionEditor::~DatabaseConnectionEditor()
{
}

void DatabaseConnectionEditor::deleteConnection()
{
    if ( listConnections->currentItem() == -1 )
	return;
    project->removeDatabaseConnection( listConnections->currentText() );
    delete listConnections->item( listConnections->currentItem() );
    if ( listConnections->count() ) {
	listConnections->setCurrentItem( 0 );
	currentConnectionChanged( listConnections->currentText() );
    } else {
	enableAll( FALSE );
    }
    project->saveConnections();
}

void DatabaseConnectionEditor::newConnection()
{
    blockChanges = TRUE;
    enableAll( TRUE );
    QString n( "(default)" );
    if ( project->databaseConnection( n ) ) {
	n = "connection";
	int i = 2;
	while ( project->databaseConnection( n + QString::number( i ) ) )
	    ++i;
	n = n + QString::number( i );
    }
    connectionWidget->editName->setText( n );
    listConnections->clearSelection();
    blockChanges = FALSE;
}

void DatabaseConnectionEditor::doConnect()
{
#ifndef QT_NO_SQL
    if ( listConnections->currentItem() == -1 ||
	 !listConnections->item( listConnections->currentItem() )->selected() ) { // new connection
	// ### do error checking for duplicated connection names
	DatabaseConnection *conn = new DatabaseConnection( project );
	conn->setName( connectionWidget->editName->text() );
	conn->setDriver( connectionWidget->comboDriver->lineEdit()->text() );
	conn->setDatabase( connectionWidget->comboDatabase->lineEdit()->text() );
	conn->setUsername( connectionWidget->editUsername->text() );
	conn->setPassword( connectionWidget->editPassword->text() );
	conn->setHostname( connectionWidget->editHostname->text() );
	if ( conn->refreshCatalog() ) {
	    project->addDatabaseConnection( conn );
	    listConnections->insertItem( conn->name() );
	    listConnections->setCurrentItem( listConnections->count() - 1 );
	    project->saveConnections();
	} else {
	    // ### error handling
	    delete conn;
	}
    } else { // sync // ### should this do something else? right now it just overwrites all info about the connection...
	DatabaseConnection *conn = project->databaseConnection( listConnections->currentText() );
	conn->setName( connectionWidget->editName->text() );
	conn->setDriver( connectionWidget->comboDriver->lineEdit()->text() );
	conn->setDatabase( connectionWidget->comboDatabase->lineEdit()->text() );
	conn->setUsername( connectionWidget->editUsername->text() );
	conn->setPassword( connectionWidget->editPassword->text() );
	conn->setHostname( connectionWidget->editHostname->text() );
	conn->refreshCatalog();
	project->saveConnections();
    }
#endif
}

void DatabaseConnectionEditor::currentConnectionChanged( const QString &s )
{
#ifndef QT_NO_SQL
    DatabaseConnection *conn = project->databaseConnection( s );
    blockChanges = TRUE;
    enableAll( (bool)conn );
    connectionWidget->editName->setEnabled( FALSE );
    blockChanges = FALSE;
    if ( !conn )
	return;
    blockChanges = TRUE;
    connectionWidget->editName->setText( conn->name() );
    blockChanges = FALSE;
    connectionWidget->comboDriver->lineEdit()->setText( conn->driver() );
    connectionWidget->comboDatabase->lineEdit()->setText( conn->database() );
    connectionWidget->editUsername->setText( conn->username() );
    connectionWidget->editPassword->setText( conn->password() );
    connectionWidget->editHostname->setText( conn->hostname() );
#endif
}

void DatabaseConnectionEditor::connectionNameChanged( const QString &s )
{
    if ( listConnections->currentItem() == 0 || blockChanges )
	return;
    listConnections->changeItem( s, listConnections->currentItem() );
}

void DatabaseConnectionEditor::enableAll( bool b )
{
    connectionWidget->editName->setEnabled( b );
    connectionWidget->editName->setText( "" );
    connectionWidget->comboDriver->setEnabled( b );
    connectionWidget->comboDriver->lineEdit()->setText( "" );
    connectionWidget->comboDatabase->setEnabled( b );
    connectionWidget->comboDatabase->clear();
    connectionWidget->editUsername->setEnabled( b );
    connectionWidget->editUsername->setText( "" );
    connectionWidget->editPassword->setEnabled( b );
    connectionWidget->editPassword->setText( "" );
    connectionWidget->editHostname->setEnabled( b );
    connectionWidget->editHostname->setText( "" );
    buttonConnect->setEnabled( b );
}
