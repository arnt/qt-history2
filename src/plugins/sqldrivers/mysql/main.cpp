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

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "../../../sql/drivers/mysql/qsql_mysql.h"

class QMYSQLDriverPlugin : public QSqlDriverPlugin
{
public:
    QMYSQLDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QMYSQLDriverPlugin::QMYSQLDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QMYSQLDriverPlugin::create(const QString &name)
{
    if (name == QLatin1String("QMYSQL3")) {
        QMYSQLDriver* driver = new QMYSQLDriver();
        return driver;
    }
    return 0;
}

QStringList QMYSQLDriverPlugin::keys() const
{
    QStringList l;
    l  << QLatin1String("QMYSQL3");
    return l;
}

Q_EXPORT_PLUGIN(QMYSQLDriverPlugin)
