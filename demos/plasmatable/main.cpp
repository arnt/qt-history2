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

#include <qapplication.h>
#include <qhbox.h>
#include <qtableview.h>
#include <qheaderview.h>
#include "plasmamodel.h"
#include "plasmadelegate.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QHBox page;

    int rc = 100;
    int cc = 160;
    QAbstractItemModel *data = new PlasmaModel(rc, cc, &page);
    QTableView *view = new QTableView(&page);
    QAbstractItemDelegate *delegate = new PlasmaDelegate(view);

    view->setModel(data);
    view->setItemDelegate(delegate);
    view->setShowGrid(false);
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();

    for (int c = 0; c < cc; ++c)
        view->resizeColumnToContents(c, false);
    for (int r = 0; r < rc; ++r)
        view->resizeRowToContents(r, false);

    app.setMainWidget(&page);

    page.resize(view->sizeHint());
    page.show();

    return app.exec();
}
