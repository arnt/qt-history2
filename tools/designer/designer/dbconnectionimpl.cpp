/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "dbconnectionimpl.h"
#include "dbconnection.h"
#include "project.h"
#include "asciivalidator.h"
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qsqldatabase.h>

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
#ifndef QT_NO_SQL
    conn->setUsername( connectionWidget->editUsername->text() );
    conn->setPassword( connectionWidget->editPassword->text() );
    conn->setHostname( connectionWidget->editHostname->text() );
#endif
    DatabaseConnectionEditorBase::accept();
}

void DatabaseConnectionEditor::init()
{
    connectionWidget->editName->setEnabled( false );
    connectionWidget->editName->setValidator( new AsciiValidator( connectionWidget->editName ) );
    connectionWidget->editName->setText( conn->name() );
    connectionWidget->comboDriver->setEnabled( false );
    connectionWidget->comboDriver->lineEdit()->setText( conn->driver() );
    connectionWidget->editDatabase->setEnabled( false );
    connectionWidget->editDatabase->setText( conn->database() );
    connectionWidget->editUsername->setEnabled( true );
    connectionWidget->editUsername->setText( conn->username() );
    connectionWidget->editPassword->setEnabled( true );
    connectionWidget->editPassword->setText( "" );
    connectionWidget->editHostname->setEnabled( true );
    connectionWidget->editHostname->setText( conn->hostname() );
    connectionWidget->editPort->setEnabled( true );
    connectionWidget->editPort->setValue( conn->port() );
    connectionWidget->editUsername->setFocus();
    connectionWidget->editUsername->selectAll();
}
