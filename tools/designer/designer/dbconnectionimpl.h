/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DBCONNECTIONIMPL_H
#define DBCONNECTIONIMPL_H

#include "dbconnectioneditor.h"

class DatabaseConnection;
class DatabaseConnectionWidget;
class QGridLayout;

class DatabaseConnectionEditor : public DatabaseConnectionEditorBase
{
    Q_OBJECT

public:
    DatabaseConnectionEditor( DatabaseConnection* connection, QWidget* parent = 0,
			       const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~DatabaseConnectionEditor();

public slots:
    void accept();
private:
    void init();

private:
    DatabaseConnection *conn;
    QGridLayout* lay;
    DatabaseConnectionWidget* connectionWidget;

};

#endif // DBCONNECTIONIMPL_H
