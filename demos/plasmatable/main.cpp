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
#include "colorfilter.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSplitter splitter;

    int rc = 100;
    int cc = 160;
 
    QAbstractItemModel *data = new PlasmaModel(rc, cc, &splitter);

    // 1st view

    QTableView *view = new QTableView(&splitter);
    QAbstractItemDelegate *delegate = new PlasmaDelegate(view);

    ColorFilter *filter = new ColorFilter(&splitter);
    filter->setModel(data);
    filter->setFilter(0x00f0f0f0);

    view->setModel(filter);
    view->setItemDelegate(delegate);
    view->setShowGrid(false);
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();

    for (int c = 0; c < cc; ++c)
        view->resizeColumnToContents(c);
    for (int r = 0; r < rc; ++r)
        view->resizeRowToContents(r);

    // 2nd view

    view = new QTableView(&splitter);
    delegate = new PlasmaDelegate(view);
    view->setModel(data);
    view->setItemDelegate(delegate);
    view->setShowGrid(false);
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();

    for (int c = 0; c < cc; ++c)
        view->resizeColumnToContents(c);
    for (int r = 0; r < rc; ++r)
        view->resizeRowToContents(r);
    
    app.setMainWidget(&splitter);
    splitter.show();

    return app.exec();
}
