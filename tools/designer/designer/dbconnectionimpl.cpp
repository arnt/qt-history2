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

#include "dbconnectionimpl.h"
#include "project.h"
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qsqldatabase.h>
#include <qmessagebox.h>
#include <qapplication.h>

DatabaseConnectionEditor::DatabaseConnectionEditor( DatabaseConnection* connection, QWidget* parent,
						    const char* name, bool modal, WFlags fl )
    : DatabaseConnectionEditorBase( parent, name, modal, fl ), conn( connection )
{
    connectionWidget = new DatabaseConnectionWidget( grp );
    grpLayout->addWidget( connectionWidget, 0, 0 );
    init();
}

DatabaseConnectionEditor::~DatabaseConnectionEditor()
{
}

void DatabaseConnectionEditor::accept()
{
    bool a = TRUE;
#ifndef QT_NO_SQL
    conn->setPassword( connectionWidget->editPassword->text() );
    conn->setUsername( connectionWidget->editUsername->text() );
    conn->setHostname( connectionWidget->editHostname->text() );
    a = conn->open();
    if ( !a ) {
	switch( QMessageBox::warning( this, tr( "Connection" ),
				      tr( "Could not connect to the database.\n"
					  "Press 'OK' to continue or 'Cancel' to specify different\n"
					  "connection information.\n" ),
				      tr( "&OK" ), tr( "&Cancel" ), 0, 0, 1 ) ) {
	case 0: // OK or Enter
	    a = TRUE;
	    break;
	case 1: // Cancel or Escape
	    a = FALSE;
	    break;
	}
    }
#endif
    if ( a )
	DatabaseConnectionEditorBase::accept();
}

void DatabaseConnectionEditor::init()
{
    connectionWidget->editName->setEnabled( FALSE );
    connectionWidget->editName->setText( conn->name() );
    connectionWidget->comboDriver->setEnabled( FALSE );
    connectionWidget->comboDriver->lineEdit()->setText( conn->driver() );
    connectionWidget->comboDatabase->setEnabled( FALSE );
    connectionWidget->comboDatabase->lineEdit()->setText( conn->database() );
    connectionWidget->editUsername->setEnabled( TRUE );
    connectionWidget->editUsername->setText( conn->username() );
    connectionWidget->editPassword->setEnabled( TRUE );
    connectionWidget->editPassword->setText( "" );
    connectionWidget->editHostname->setEnabled( TRUE );
    connectionWidget->editHostname->setText( conn->hostname() );
    connectionWidget->editUsername->setFocus();
    connectionWidget->editUsername->selectAll();
}
