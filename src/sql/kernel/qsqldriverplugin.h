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

#ifndef QSQLDRIVERPLUGIN_H
#define QSQLDRIVERPLUGIN_H

#ifndef QT_H
#include "qplugin.h"
#include "qfactoryinterface.h"
#endif // QT_H

#ifndef QT_NO_SQL
class QSqlDriver;

struct Q_SQL_EXPORT QSqlDriverFactoryInterface : public QFactoryInterface
{
    virtual QSqlDriver* create(const QString& name) = 0;
};

Q_DECLARE_INTERFACE(QSqlDriverFactoryInterface, "http://trolltech.com/Qt/QSqlDriverFactoryInterface")

class Q_SQL_EXPORT QSqlDriverPlugin : public QObject, public QSqlDriverFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QSqlDriverFactoryInterface:QFactoryInterface)
public:
    QSqlDriverPlugin(QObject *parent = 0);
    ~QSqlDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QSqlDriver *create(const QString &key) = 0;

};

#endif // QT_NO_SQL

#endif // QSQLDRIVERPLUGIN_H
