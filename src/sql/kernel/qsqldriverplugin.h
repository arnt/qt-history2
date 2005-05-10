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

#include "QtCore/qplugin.h"
#include "QtCore/qfactoryinterface.h"

class QSqlDriver;

struct Q_SQL_EXPORT QSqlDriverFactoryInterface : public QFactoryInterface
{
    virtual QSqlDriver *create(const QString &name) = 0;
};

Q_DECLARE_INTERFACE(QSqlDriverFactoryInterface, "com.trolltech.Qt.QSqlDriverFactoryInterface")

class Q_SQL_EXPORT QSqlDriverPlugin : public QObject, public QSqlDriverFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QSqlDriverFactoryInterface:QFactoryInterface)
public:
    explicit QSqlDriverPlugin(QObject *parent = 0);
    ~QSqlDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QSqlDriver *create(const QString &key) = 0;

};

#endif // QSQLDRIVERPLUGIN_H
