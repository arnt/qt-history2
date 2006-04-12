#include <QtGui>
#include <QApplication>
#include <QSortFilterProxyModel>

class MyItemModel : public QStandardItemModel
{
public:
    MyItemModel(QWidget *parent = 0);
};

MyItemModel::MyItemModel(QWidget *parent)
    : QStandardItemModel(parent)
{};

class Widget : public QWidget
{
public:
    Widget(QWidget *parent = 0);
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
        QTreeView *treeView = new QTreeView;
        MyItemModel *model = new MyItemModel(this);

        treeView->setModel(model);

        MyItemModel *sourceModel = new MyItemModel(this);
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);

        proxyModel->setSourceModel(sourceModel);
        treeView->setModel(proxyModel);

        treeView->setSortingEnabled(true);

        proxyModel->sort(2, Qt::AscendingOrder);
        proxyModel->setFilterRegExp(QRegExp(".png", Qt::CaseInsensitive,
                                            QRegExp::FixedString));
        proxyModel->setFilterKeyColumn(1);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget widget;
    widget.show();
    return app.exec();
}




class MySortFilterProxyModel : public QSortFilterProxyModel
{
public:
    MySortFilterProxyModel( QObject * parent = 0 );

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool dateInRange(QDate date) const;
};

MySortFilterProxyModel::MySortFilterProxyModel(QObject * parent)
    : QSortFilterProxyModel(parent)
{};

bool MySortFilterProxyModel::lessThan(const QModelIndex &left,
                                              const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if (leftData.type() == QVariant::DateTime) {
         return leftData.toDateTime() < rightData.toDateTime();
    } else {
         return QString::localeAwareCompare(leftData.toString(),
                                               rightData.toString()) < 0;
    }
}

bool MySortFilterProxyModel::filterAcceptsRow(int sourceRow,
                                             const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
    QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent);

     return (sourceModel()->data(index0).toString().contains(filterRegExp())
             || sourceModel()->data(index1).toString().contains(filterRegExp()))
            && dateInRange(sourceModel()->data(index2).toDate());
}

bool MySortFilterProxyModel::dateInRange(QDate date) const
{
    return true;
}
