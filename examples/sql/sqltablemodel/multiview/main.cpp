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

/* This examples demonstrates how to use a QSqlTableModel with
   several views.
 */

#include <qapplication.h>
#include <qgenerictableview.h>
#include <qhbox.h>
#include <qsqltablemodel.h>

#include "../../connection.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    createConnection();

    QSqlTableModel *model = new QSqlTableModel(&app);
    model->setTable("persons");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    QGenericTableView view1(model);
    QGenericTableView view2(model);

    view1.show();
    view2.show();

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    return app.exec();
}

