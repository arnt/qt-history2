#include <QtGui>
#include <QtSql>

#include "../connection.h"

void initializeModel(QSqlTableModel *model)
{
    model->setTable("person");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("First name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Last name"));
}

QTableView *createView(const QString &title, QSqlTableModel *model)
{
    QTableView *view = new QTableView;
    view->setModel(model);
    view->setWindowTitle(title);
    return view;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (!createConnection())
        return 1;

    QSqlTableModel model;

    initializeModel(&model);

    QTableView *view1 = createView(QObject::tr("Table Model (View 1)"), &model);
    QTableView *view2 = createView(QObject::tr("Table Model (View 2)"), &model);

    view1->show();
    view2->move(view1->x() + view1->width() + 20, view1->y());
    view2->show();

    return app.exec();
}
