/****************************************************************************
**
** Implementation of OCI driver plugin.
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
#include "../../../sql/drivers/oci/qsql_oci.h"

class QOCIDriverPlugin : public QSqlDriverPlugin
{
public:
    QOCIDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys();
};

QOCIDriverPlugin::QOCIDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QOCIDriverPlugin::create(const QString &name)
{
    if (name == "QOCI8") {
        QOCIDriver* driver = new QOCIDriver();
        return driver;
    }
    return 0;
}

QStringList QOCIDriverPlugin::keys()
{
    QStringList l;
    l.append("QOCI8");
    return l;
}

Q_EXPORT_PLUGIN(QOCIDriverPlugin)
