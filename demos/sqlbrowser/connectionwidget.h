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

#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <qabstractitemmodel.h>
#include <qvector.h>
#include <qstringlist.h>
#include <qwidget.h>

class QTreeView;
class QSqlDatabase;

class ConnectionModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    ConnectionModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = DisplayRole) const;
    bool hasChildren(const QModelIndex &parent) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const;
    QModelIndex parent(const QModelIndex &child) const;

    bool isActiveConnection(const QModelIndex &index) const;

public slots:
    void refresh();

private:
    struct CData { QString cname, label; };
    QVector<CData> connections;
    QList<QStringList> tableNames;
};

class ConnectionWidget: public QWidget
{
    Q_OBJECT
public:
    ConnectionWidget(QWidget *parent = 0);
    virtual ~ConnectionWidget();

    QSqlDatabase currentDatabase() const;

public slots:
    void refresh();

private:
    QTreeView *tree;
    ConnectionModel *model;
};

#endif

