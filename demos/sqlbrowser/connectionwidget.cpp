#include "connectionwidget.h"

#include <qsqldatabase.h>
#include <qgenericheader.h>
#include <qgenerictreeview.h>
#include <qlayout.h>

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

bool ConnectionModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.data() || parent.row() >= connections.count() || parent.column() > 0)
        return false;
    return !tableNames.at(parent.row()).isEmpty();
}

QVariant ConnectionModel::data(const QModelIndex &index, int role) const
{
    if (role != Role_Display || index.column() > 0 || index.row() < 0)
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
        return QAbstractItemModel::index(row, column, parent, type);
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

