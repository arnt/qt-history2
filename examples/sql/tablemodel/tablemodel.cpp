#include <QtGui>
#include <QtSql>

#include "../connection.h"

void createView(const QString &title, QSqlTableModel *model)
{
    QTableView *view = new QTableView;
    view->setModel(model);
    view->setWindowTitle(title);
    view->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (!createConnection())
        return 1;

    QSqlTableModel model;
    model.setTable("person");
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);
    model.select();

    createView(QObject::tr("Table Model (View 1)"), &model);
    createView(QObject::tr("Table Model (View 2)"), &model);

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
