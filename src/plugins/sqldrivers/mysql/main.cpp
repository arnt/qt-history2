/****************************************************************************
**
** Implementation of MySQL driver plugin.
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
#include "../../../sql/drivers/mysql/qsql_mysql.h"

class QMYSQLDriverPlugin : public QSqlDriverPlugin
{
public:
    QMYSQLDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys();
};

QMYSQLDriverPlugin::QMYSQLDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QMYSQLDriverPlugin::create(const QString &name)
{
    if (name == "QMYSQL3") {
        QMYSQLDriver* driver = new QMYSQLDriver();
        return driver;
    }
    return 0;
}

QStringList QMYSQLDriverPlugin::keys()
{
    QStringList l;
    l  << "QMYSQL3";
    return l;
}

Q_EXPORT_PLUGIN(QMYSQLDriverPlugin)
