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

/* This examples demonstrates how to use QSqlTableModel to display
   a database table. The QSQLITE driver is required for this example.
 */

#include <qapplication.h>
#include <qgenerictableview.h>
#include <qsqltablemodel.h>

#include "../../connection.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    createConnection();

    QSqlTableModel *model = new QSqlTableModel(&app);
    model->setTable("persons");
    model->setFilter("id > 0");
    model->select();

    QGenericTableView view(model, 0);
    app.setMainWidget(&view);
    view.show();

    return app.exec();
}

