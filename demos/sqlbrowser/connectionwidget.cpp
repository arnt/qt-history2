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

#include "connectionwidget.h"

#include <qsqldatabase.h>
#include <qgenericheader.h>
#include <qgenerictreeview.h>
#include <qlayout.h>

#include <qitemdelegate.h>
#include <qpainter.h>

class ConnectionModelDelegate: public QItemDelegate
{
public:
    ConnectionModelDelegate(QObject *parent): QItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QAbstractItemModel *model, const QModelIndex &index) const
    {
        const ConnectionModel *cm = static_cast<const ConnectionModel *>(model);
        if (cm->isActiveConnection(index)) {
            QFont fnt = painter->font();
            QFont boldFnt = fnt;
            boldFnt.setBold(true);
            painter->setFont(boldFnt);
            QItemDelegate::paint(painter, option, model, index);
            painter->setFont(fnt);
        } else {
            QItemDelegate::paint(painter, option, model, index);
        }
    }
};


ConnectionModel::ConnectionModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    refresh();
}

int ConnectionModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return connections.count();
    if (parent.data() || parent.row() >= tableNames.count())
        return 0;
    return tableNames.at(parent.row()).count();
}

int ConnectionModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

bool ConnectionModel::isActiveConnection(const QModelIndex &index) const
{
    return index.row() == 0 && index.column() == 0 && !index.data()
           && index.type() == QModelIndex::View;
}

bool ConnectionModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.data() || parent.row() >= connections.count() || parent.column() > 0)
        return false;
    return !tableNames.at(parent.row()).isEmpty();
}

QVariant ConnectionModel::data(const QModelIndex &index, int role) const
{
    if (role != DisplayRole || index.column() > 0 || index.row() < 0)
        return QVariant();

    if (index.type() == QModelIndex::View) {
        if (index.data()) {
            int row = reinterpret_cast<int>(index.data()) - 1;
            if (row >= tableNames.count())
                return QVariant();
            return tableNames.at(row).value(index.row());
        }
        if (index.row() >= connections.count())
            return QVariant();
        return connections.at(index.row()).label;
    }

    if (index.type() == QModelIndex::HorizontalHeader)
        return QString("Connections");

    return QVariant();
}

QModelIndex ConnectionModel::index(int row, int column, const QModelIndex &parent,
                                   QModelIndex::Type type) const
{
    if (!parent.isValid())
        return createIndex(row, column, 0, type);
    if (parent.data() || parent.row() >= connections.count())
        return QModelIndex();
    return createIndex(row, column, reinterpret_cast<void*>(parent.row() + 1), type);
}

void ConnectionModel::refresh()
{
    QStringList list = QSqlDatabase::connectionNames();
    if (!connections.isEmpty()) {
        emit rowsRemoved(QModelIndex(), 0, connections.count() - 1);
        connections.clear();
        tableNames.clear();
    }

    for (int i = 0; i < list.count(); ++i) {
        QString cn = list.at(i);
        QSqlDatabase db = QSqlDatabase::database(cn);
        CData data;
        data.cname = cn;
        data.label = db.driverName() + "." + db.databaseName();
        connections.append(data);
        tableNames.append(db.tables());
    }

    if (!connections.isEmpty())
        emit rowsInserted(QModelIndex(), 0, connections.count() - 1);
}

QModelIndex ConnectionModel::parent(const QModelIndex &child) const
{
    if (child.data())
        return index(reinterpret_cast<int>(child.data()) - 1, 0);
    return QModelIndex();
}

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    tree = new QGenericTreeView(this);
    tree->setItemDelegate(new ConnectionModelDelegate(this));
    model = new ConnectionModel(tree);
    tree->setModel(model);
    tree->header()->setResizeMode(QGenericHeader::Stretch);
    layout->addWidget(tree);
}

ConnectionWidget::~ConnectionWidget()
{
}

void ConnectionWidget::refresh()
{
    model->refresh();
}

QSqlDatabase ConnectionWidget::currentDatabase() const
{
    return QSqlDatabase::database(QSqlDatabase::connectionNames().first());
}

