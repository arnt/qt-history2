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
