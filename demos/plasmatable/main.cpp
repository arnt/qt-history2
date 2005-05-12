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
#include <qsplitter.h>
#include <qtableview.h>
#include <qheaderview.h>

#include "plasmamodel.h"
#include "plasmadelegate.h"
#include "hexdelegate.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSplitter splitter;

    int rc = 100;
    int cc = 100;

    QAbstractItemModel *data = new PlasmaModel(rc, cc, &splitter);
    QItemSelectionModel *selections = new QItemSelectionModel(data);

    // 1st view
    QTableView *plasmaView = new QTableView(&splitter);
    QAbstractItemDelegate *delegate = new PlasmaDelegate(plasmaView);
    plasmaView->setModel(data);
    plasmaView->setItemDelegate(delegate);
    plasmaView->setSelectionModel(selections);
    plasmaView->setShowGrid(false);
    plasmaView->horizontalHeader()->hide();
    plasmaView->verticalHeader()->hide();

    // 2nd view
    QTableView *hexView = new QTableView(&splitter);
    hexView->setModel(data);
    hexView->setItemDelegate(new HexDelegate(hexView));
    hexView->setSelectionModel(selections);

    for (int c = 0; c < cc; ++c) {
        plasmaView->resizeColumnToContents(c);
        hexView->resizeColumnToContents(c);
    }

    for (int r = 0; r < rc; ++r) {
        plasmaView->resizeRowToContents(r);
        hexView->resizeRowToContents(r);
    }

    splitter.resize(800, 450);
    splitter.show();

    return app.exec();
}
