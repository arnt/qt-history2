/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "../connection.h"
#include "customsqlmodel.h"
#include "editablesqlmodel.h"

void initializeModel(QSqlQueryModel *model)
{
    model->setQuery("select * from person");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("First name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Last name"));
}

void createView(const QString &title, QSqlQueryModel *model)
{
    static int offset = 0;

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setWindowTitle(title);
    view->move(100 + offset, 100 + offset);
    offset += 20;
    view->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (!createConnection())
        return 1;

    QSqlQueryModel plainModel;
    EditableSqlModel editableModel;
    CustomSqlModel customModel;

    initializeModel(&plainModel);
    initializeModel(&editableModel);
    initializeModel(&customModel);

    createView(QObject::tr("Plain Query Model"), &plainModel);
    createView(QObject::tr("Editable Query Model"), &editableModel);
    createView(QObject::tr("Custom Query Model"), &customModel);

    return app.exec();
}
