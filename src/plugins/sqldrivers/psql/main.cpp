/****************************************************************************
**
** Implementation of PostgreSQL driver plugin.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qsqldriverplugin.h>
#include "../../../sql/drivers/psql/qsql_psql.h"

class QPSQLDriverPlugin : public QSqlDriverPlugin
{
public:
    QPSQLDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys();
};

QPSQLDriverPlugin::QPSQLDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QPSQLDriverPlugin::create(const QString &name)
{
    if (name == "QPSQL7") {
        QPSQLDriver* driver = new QPSQLDriver();
        return driver;
    }
    return 0;
}

QStringList QPSQLDriverPlugin::keys()
{
    QStringList l;
    l.append("QPSQL7");
    return l;
}

Q_EXPORT_PLUGIN(QPSQLDriverPlugin)
