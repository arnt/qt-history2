/****************************************************************************
**
** Definition of QSqlDriverPlugin class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLDRIVERPLUGIN_H
#define QSQLDRIVERPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_SQL
#ifndef QT_NO_COMPONENT

class QSqlDriver;
class QSqlDriverPluginPrivate;

class Q_SQL_EXPORT QSqlDriverPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QSqlDriverPlugin();
    ~QSqlDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QSqlDriver *create( const QString &key ) = 0;

private:
    QSqlDriverPluginPrivate *d;
};

#endif // QT_NO_COMPONENT
#endif // QT_NO_SQL

#endif // QSQLDRIVERPLUGIN_H
