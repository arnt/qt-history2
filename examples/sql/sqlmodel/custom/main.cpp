/****************************************************************************
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is an example program for the Qt SQL module.
 ** EDITIONS: NOLIMITS
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

/* This examples demonstrates how to subclass QSqlModel to display
   custom values. The QSQLITE driver is required for this example.
 */

#include <qapplication.h>
#include <qgenerictableview.h>
#include <qsqlmodel.h>

#include "../../connection.h"

class CustomSqlModel: public QSqlModel
{
public:
    CustomSqlModel(QObject *parent = 0): QSqlModel(parent) {}
    QVariant data(const QModelIndex &item, int role = QAbstractItemModel::Role_Display) const;
};

QVariant CustomSqlModel::data(const QModelIndex &item, int role) const
{
    QVariant v = QSqlModel::data(item, role);
    if (v.isValid() && item.column() == 0
        && item.type() == QModelIndex::View && role == QAbstractItemModel::Role_Display)
        return v.toString().prepend("#");
    return v;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    createConnection();

    CustomSqlModel *model = new CustomSqlModel(&app);
    model->setQuery("select * from persons");

    QGenericTableView view(model, 0);
    app.setMainWidget(&view);
    view.show();

    return app.exec();
}

