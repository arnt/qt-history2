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

#define Q_UUIDIMPL
#include <qsqldriverplugin.h>
#include <qstringlist.h>
#ifdef Q_OS_WIN32    // We assume that MS SQL Server is used. Set Q_USE_SYBASE to force Sybase.
#include <windows.h>
#endif
#include "../../../sql/drivers/tds/qsql_tds.h"


class QTDSDriverPlugin : public QSqlDriverPlugin
{
public:
    QTDSDriverPlugin();

    QSqlDriver* create(const QString &);
    QStringList keys() const;
};

QTDSDriverPlugin::QTDSDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QTDSDriverPlugin::create(const QString &name)
{
    if (name == QLatin1String("QTDS7")) {
        QTDSDriver* driver = new QTDSDriver();
        return driver;
    }
    return 0;
}

QStringList QTDSDriverPlugin::keys() const
{
    QStringList l;
    l.append(QLatin1String("QTDS7"));
    return l;
}

Q_EXPORT_PLUGIN(QTDSDriverPlugin)
