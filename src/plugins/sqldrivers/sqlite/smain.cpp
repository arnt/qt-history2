/****************************************************************************
**
** Implementation of SQLite driver plugin.
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
#include "../../../../src/sql/drivers/sqlite/qsql_sqlite.h"

class QSQLiteDriverPlugin : public QSqlDriverPlugin
{
public:
    QSQLiteDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QSQLiteDriverPlugin::QSQLiteDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QSQLiteDriverPlugin::create(const QString &name)
{
    if (name == "QSQLITE") {
        QSQLiteDriver* driver = new QSQLiteDriver();
        return driver;
    }
    return 0;
}

QStringList QSQLiteDriverPlugin::keys() const
{
    QStringList l;
    l  << "QSQLITE";
    return l;
}

Q_EXPORT_PLUGIN(QSQLiteDriverPlugin)
