/****************************************************************************
**
** Implementation of DB2 driver plugin.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "../../../sql/drivers/db2/qsql_db2.h"

class QDB2DriverPlugin : public QSqlDriverPlugin
{
public:
    QDB2DriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QDB2DriverPlugin::QDB2DriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QDB2DriverPlugin::create(const QString &name)
{
    if (name == "QDB2") {
        QDB2Driver* driver = new QDB2Driver();
        return driver;
    }
    return 0;
}

QStringList QDB2DriverPlugin::keys() const
{
    QStringList l;
    l.append("QDB2");
    return l;
}

Q_EXPORT_PLUGIN(QDB2DriverPlugin)
