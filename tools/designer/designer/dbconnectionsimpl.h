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

#ifndef DBCONNECTIONSIMPL_H
#define DBCONNECTIONSIMPL_H

#include "dbconnections.h"
#include "dbconnection.h"

class Project;

class DatabaseConnectionsEditor : public DatabaseConnectionBase
{
    Q_OBJECT

public:
    DatabaseConnectionsEditor( Project *pro, QWidget* parent = 0,
			       const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~DatabaseConnectionsEditor();

protected slots:
    void deleteConnection();
    void newConnection();
    void doConnect();
    void currentConnectionChanged( const QString & );
    void connectionNameChanged( const QString &s );

private:
    void enableAll( bool b );

private:
    Project *project;
    DatabaseConnectionWidget* connectionWidget;

};

#endif // DBCONNECTIONSIMPL_H
