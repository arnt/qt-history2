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
#include "../../../sql/drivers/ibase/qsql_ibase.h"

class QIBaseDriverPlugin : public QSqlDriverPlugin
{
public:
    QIBaseDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QIBaseDriverPlugin::QIBaseDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QIBaseDriverPlugin::create(const QString &name)
{
    if (name == QLatin1String("QIBASE")) {
        QIBaseDriver* driver = new QIBaseDriver();
        return driver;
    }
    return 0;
}

QStringList QIBaseDriverPlugin::keys() const
{
    QStringList l;
    l  << QLatin1String("QIBASE");
    return l;
}

Q_EXPORT_PLUGIN(QIBaseDriverPlugin)
