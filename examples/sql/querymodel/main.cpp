#include <QtGui>
#include <QtSql>

#include "../connection.h"

#include "customsqlmodel.h"
#include "editablesqlmodel.h"

void createView(const QString &title, QSqlQueryModel *model)
{
    model->setQuery("select * from persons");

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setWindowTitle(title);
    view->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    createConnection();

    QSqlQueryModel plainModel;
    EditableSqlModel editableModel;
    CustomSqlModel customModel;

    createView(QObject::tr("Plain Query Model"), &plainModel);
    createView(QObject::tr("Editable Query Model"), &editableModel);
    createView(QObject::tr("Custom Query Model"), &customModel);

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
