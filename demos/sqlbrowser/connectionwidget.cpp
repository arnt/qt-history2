#include "connectionwidget.h"

#include <qsqldatabase.h>
#include <qgenerictreeview.h>
#include <qlayout.h>

ConnectionModel::ConnectionModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    refresh();
}

int ConnectionModel::rowCount(const QModelIndex &parent) const
{
    return connections.count();
}

int ConnectionModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant ConnectionModel::data(const QModelIndex &index, int role = Role_Display) const
{
    if (role != Role_Display)
        return QVariant();

    if (index.parent().isValid()) {
        return QVariant();
    }

    if (index.type() == QModelIndex::Data)
        return QVariant();
}

void ConnectionModel::refresh()
{
    QStringList list = QSqlDatabase::connectionNames();
    if (!connections.isEmpty()) {
        emit rowsRemoved(QModelIndex(), 0, connections.count());
        connections.clear();
    }

    for (int i = 0; i < list.count(); ++i) {
        QString cn = list.at(i);
        QSqlDatabase db = QSqlDatabase::database(cn);
        connections[cn] = db.driverName();
    }

    if (!connections.isEmpty())
        emit rowsInserted(QModelIndex(), 0, connections.count());
}

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    tree = new QGenericTreeView(this);
    model = new ConnectionModel(tree);
    tree->setModel(model);
    layout->addWidget(tree);
}

void ConnectionWidget::refresh()
{
    model->refresh();
}


