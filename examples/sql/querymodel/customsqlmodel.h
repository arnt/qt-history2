/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CUSTOMSQLMODEL_H
#define CUSTOMSQLMODEL_H

#include <QSqlQueryModel>

class CustomSqlModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    CustomSqlModel(QObject *parent = 0);

    QVariant data(const QModelIndex &item, int role) const;
};

#endif
