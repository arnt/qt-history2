/****************************************************************************
**
** Implementation of ODBC driver plugin.
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
#include "../../../sql/drivers/odbc/qsql_odbc.h"

class QODBCDriverPlugin : public QSqlDriverPlugin
{
public:
    QODBCDriverPlugin();

    QSqlDriver* create( const QString & );
    QStringList keys() const;
};

QODBCDriverPlugin::QODBCDriverPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QODBCDriverPlugin::create( const QString &name )
{
    if ( name == "QODBC3" ) {
	QODBCDriver* driver = new QODBCDriver();
	return driver;
    }
    return 0;
}

QStringList QODBCDriverPlugin::keys() const
{
    QStringList l;
    l.append("QODBC3");
    return l;
}

Q_EXPORT_PLUGIN( QODBCDriverPlugin )
