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

#include <QtTest/QtTest>
#include <QtCore/QtCore>
#include <QtGui/QtGui>

/*
    To add a view to be tested add the header file to the includes
    and impliment what is needed in the functions below.

    You can add more then one view, several Qt views are included as examples.

    In tst_qitemview.cpp a new ViewsToTest object is created for each test.

    When you have errors fix the first ones first.  Later tests depend upon them working
*/

class ViewsToTest
{
public:
    ViewsToTest();

    QAbstractItemView *createView(const QString &viewType);
    QModelIndex hiddenIndex(QAbstractItemView *view);
    
    enum Display { DisplayNone, DisplayRoot };

    struct test {
        test(QString m, Display d) : viewType(m), display(d){};
        QString viewType;
        Display display;
    };

    QList<test> tests;
};


/*!
    Add new tests, they can be the same view, but in a different state.
 */
ViewsToTest::ViewsToTest()
{
    tests.append(test("QTreeView_ScrollPerItem", DisplayRoot));
    tests.append(test("QTreeView_ScrollPerPixel", DisplayRoot));
    tests.append(test("QListView_ScrollPerItem", DisplayRoot));
    tests.append(test("QListView_ScrollPerPixel", DisplayRoot));
    tests.append(test("QHeaderViewHorizontal", DisplayNone));
    tests.append(test("QHeaderViewVertical", DisplayNone));
    tests.append(test("QTableView_ScrollPerItem", DisplayRoot));
    tests.append(test("QTableView_ScrollPerPixel", DisplayRoot));
    tests.append(test("QTableViewNoGrid", DisplayRoot));
}

/*!
    Return a new viewType.
 */
QAbstractItemView *ViewsToTest::createView(const QString &viewType)
{
    QAbstractItemView *view = 0;
    if (viewType == "QListView_ScrollPerItem") {
        view = new QListView();
        view->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
        view->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    } else if (viewType == "QListView_ScrollPerPixel") {
        view = new QListView();
        view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    } else if (viewType == "QHeaderViewHorizontal") {
        view = new QHeaderView(Qt::Horizontal);
    } else if (viewType == "QHeaderViewVertical") {
        view = new QHeaderView(Qt::Vertical);
    } else if (viewType == "QTableView_ScrollPerItem") {
        view = new QTableView();
        view->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
        view->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    } else if (viewType == "QTableView_ScrollPerPixel") {
        view = new QTableView();
        view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    } else if (viewType == "QTableViewNoGrid") {
        QTableView *table = new QTableView();
        table->setShowGrid(false);
        view = table;
    } else if (viewType == "QTreeView_ScrollPerItem") {
        view = new QTreeView();
        view->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
        view->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    } else if (viewType == "QTreeView_ScrollPerPixel") {
        view = new QTreeView();
        view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    }
    Q_ASSERT(view);
    return view;
}

/*!
    Returns a hidden index or QModelIndex() if the view doesn't support them
 */
QModelIndex ViewsToTest::hiddenIndex(QAbstractItemView *view)
{
    if (QTableView *tableView = qobject_cast<QTableView *>(view)) {
        tableView->setColumnHidden(1, true);
        return tableView->model()->index(0, 1);
    }
    if (QTreeView *treeView = qobject_cast<QTreeView *>(view)) {
        treeView->setColumnHidden(1, true);
        return treeView->model()->index(0, 1);
    }
    if (QListView *listView = qobject_cast<QListView *>(view)) {
        listView->setRowHidden(1, true);
        return listView->model()->index(1, 0);
    }
    return QModelIndex();
}

