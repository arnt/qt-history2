/****************************************************************************
**
** Implementation of PostgreSQL driver plugin.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qsqldriverplugin.h>
#include "../../../../src/sql/drivers/psql/qsql_psql.h"

class QPSQLDriverPlugin : public QSqlDriverPlugin
{
public:
    QPSQLDriverPlugin();

    QSqlDriver* create( const QString & );
    QStringList keys() const;
};

QPSQLDriverPlugin::QPSQLDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QPSQLDriverPlugin::create( const QString &name )
{
    if ( name == "QPSQL7" ) {
	QPSQLDriver* driver = new QPSQLDriver();
	return driver;
    }
    return 0;
}

QStringList QPSQLDriverPlugin::keys() const
{
    QStringList l;
    l.append("QPSQL7");
    return l;
}

Q_EXPORT_PLUGIN( QPSQLDriverPlugin )
