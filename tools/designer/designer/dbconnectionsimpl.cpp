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

/*
 *  Constructs a DatabaseConnection which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DatabaseConnection::DatabaseConnection( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : DatabaseConnectionBase( parent, name, modal, fl )
{
}

DatabaseConnection::~DatabaseConnection()
{
}

void DatabaseConnection::deleteConnection()
{
    qWarning( "DatabaseConnection::deleteConnection() not yet implemented!" );
}

void DatabaseConnection::newConnection()
{
    qWarning( "DatabaseConnection::newConnection() not yet implemented!" );
}

void DatabaseConnection::doConnect()
{
    qWarning( "DatabaseConnection::doConnect() not yet implemented!" );
}

