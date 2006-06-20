/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qeventloop.h>
#include <qlist.h>

#include <qlistwidget.h>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qlistwidget.h gui/itemviews/qlistwidget.cpp

class tst_QListWidget : public QObject
{
    Q_OBJECT

public:
    tst_QListWidget();
    ~tst_QListWidget();

    enum ModelChanged {
        RowsAboutToBeInserted,
        RowsInserted,
        RowsAboutToBeRemoved,
        RowsRemoved,
        ColumnsAboutToBeInserted,
        ColumnsInserted,
        ColumnsAboutToBeRemoved,
        ColumnsRemoved
    };

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void itemAssignment();
    void item_data();
    void item();
    void takeItem_data();
    void takeItem();
    void setItemHidden();
    void selectedItems_data();
    void selectedItems();
    void insertItem_data();
    void insertItem();
    void insertItems_data();
    void insertItems();
    void removeItems_data();
    void removeItems();
    void itemStreaming_data();
    void itemStreaming();
    void sortItems_data();
    void sortItems();
    void closeEditor();
    void setData_data();
    void setData();
#if QT_VERSION >= 0x040200
    void insertItemsWithSorting_data();
    void insertItemsWithSorting();
    void changeDataWithSorting_data();
    void changeDataWithSorting();
#endif

protected slots:
    void rowsAboutToBeInserted(const QModelIndex &parent, int first, int last)
        { modelChanged(RowsAboutToBeInserted, parent, first, last); }
    void rowsInserted(const QModelIndex &parent, int first, int last)
        { modelChanged(RowsInserted, parent, first, last); }
    void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
        { modelChanged(RowsAboutToBeRemoved, parent, first, last); }
    void rowsRemoved(const QModelIndex &parent, int first, int last)
        { modelChanged(RowsRemoved, parent, first, last); }
    void columnsAboutToBeInserted(const QModelIndex &parent, int first, int last)
        { modelChanged(ColumnsAboutToBeInserted, parent, first, last); }
    void columnsInserted(const QModelIndex &parent, int first, int last)
        { modelChanged(ColumnsInserted, parent, first, last); }
    void columnsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
        { modelChanged(ColumnsAboutToBeRemoved, parent, first, last); }
    void columnsRemoved(const QModelIndex &parent, int first, int last)
        { modelChanged(ColumnsRemoved, parent, first, last); }

    void modelChanged(ModelChanged change, const QModelIndex &parent, int first, int last);

private:
    QListWidget *testWidget;
    QVector<QModelIndex> rcParent;
    QVector<int> rcFirst;
    QVector<int> rcLast;
};

// Testing get/set functions
void tst_QListWidget::getSetCheck()
{
    QListWidget obj1;
    QListWidgetItem *var1 = new QListWidgetItem(&obj1);
    new QListWidgetItem(&obj1);
    // QListWidgetItem * QListWidget::currentItem()
    // void QListWidget::setCurrentItem(QListWidgetItem *)
    obj1.setCurrentItem(var1);
    QCOMPARE(var1, obj1.currentItem());
#if QT_VERSION >= 0x040200
    // Itemviews in Qt < 4.2 have asserts for this. Qt >= 4.2 should handle this gracefully
    obj1.setCurrentItem((QListWidgetItem *)0);
    QCOMPARE((QListWidgetItem *)0, obj1.currentItem());
#endif
    // int QListWidget::currentRow()
    // void QListWidget::setCurrentRow(int)
    obj1.setCurrentRow(0);
    QCOMPARE(0, obj1.currentRow());
    obj1.setCurrentRow(INT_MIN);
    QCOMPARE(-1, obj1.currentRow()); // Model contains no INT_MIN row => -1 (invalid index)
    obj1.setCurrentRow(INT_MAX);
    QCOMPARE(-1, obj1.currentRow()); // Model contains no INT_MAX row => -1 (invalid index)
    obj1.setCurrentRow(1);
    QCOMPARE(1, obj1.currentRow());
}

typedef QList<int> IntList;
Q_DECLARE_METATYPE(IntList)
Q_DECLARE_METATYPE(QVariantList)

    tst_QListWidget::tst_QListWidget(): testWidget(0), rcParent(8), rcFirst(8,0), rcLast(8,0)
{
}

tst_QListWidget::~tst_QListWidget()
{
}

void tst_QListWidget::initTestCase()
{
    testWidget = new QListWidget();
    testWidget->show();

    connect(testWidget->model(), SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
            this, SLOT(rowsAboutToBeInserted(QModelIndex, int, int)));
    connect(testWidget->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(rowsInserted(QModelIndex, int, int)));
    connect(testWidget->model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
            this, SLOT(rowsAboutToBeRemoved(QModelIndex, int, int)));
    connect(testWidget->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)),
            this, SLOT(rowsRemoved(QModelIndex, int, int)));

    connect(testWidget->model(), SIGNAL(columnsAboutToBeInserted(QModelIndex, int, int)),
            this, SLOT(columnsAboutToBeInserted(QModelIndex, int, int)));
    connect(testWidget->model(), SIGNAL(columnsInserted(QModelIndex, int, int)),
            this, SLOT(columnsInserted(QModelIndex, int, int)));
    connect(testWidget->model(), SIGNAL(columnsAboutToBeRemoved(QModelIndex, int, int)),
            this, SLOT(columnsAboutToBeRemoved(QModelIndex, int, int)));
    connect(testWidget->model(), SIGNAL(columnsRemoved(QModelIndex, int, int)),
            this, SLOT(columnsRemoved(QModelIndex, int, int)));
}

void tst_QListWidget::cleanupTestCase()
{
    delete testWidget;
}

void tst_QListWidget::init()
{
    testWidget->clear();
}

void tst_QListWidget::cleanup()
{
}

void tst_QListWidget::itemAssignment()
{
    QListWidgetItem itemInWidget("inWidget", testWidget);
    itemInWidget.setFlags(itemInWidget.flags() | Qt::ItemIsTristate);
    QListWidgetItem itemOutsideWidget("outsideWidget");

    QVERIFY(itemInWidget.listWidget());
    QCOMPARE(itemInWidget.text(), QString("inWidget"));
    QVERIFY(itemInWidget.flags() & Qt::ItemIsTristate);

    QVERIFY(!itemOutsideWidget.listWidget());
    QCOMPARE(itemOutsideWidget.text(), QString("outsideWidget"));
    QVERIFY(!(itemOutsideWidget.flags() & Qt::ItemIsTristate));

    itemOutsideWidget = itemInWidget;
    QVERIFY(!itemOutsideWidget.listWidget());
    QCOMPARE(itemOutsideWidget.text(), QString("inWidget"));
    QVERIFY(itemOutsideWidget.flags() & Qt::ItemIsTristate);
}

void tst_QListWidget::item_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<bool>("outOfBounds");

    QTest::newRow("First item, row: 0") << 0 << false;
    QTest::newRow("Middle item, row: 1") << 1 << false;
    QTest::newRow("Last item, row: 2") << 2 << false;
    QTest::newRow("Out of bounds, row: -1") << -1 << true;
    QTest::newRow("Out of bounds, row: 3") << 3 << true;
}

void tst_QListWidget::item()
{
    QFETCH(int, row);
    QFETCH(bool, outOfBounds);

    (new QListWidgetItem(testWidget))->setText("item0");
    (new QListWidgetItem(testWidget))->setText("item1");
    (new QListWidgetItem(testWidget))->setText("item2");

    QCOMPARE(testWidget->count(), 3);

    QListWidgetItem *item = testWidget->item(row);
    if (outOfBounds) {
        QCOMPARE(item, static_cast<QListWidgetItem*>(0));
        QCOMPARE(testWidget->count(), 3);
    } else {
        QCOMPARE(item->text(), QString("item%1").arg(row));
        QCOMPARE(testWidget->count(), 3);
    }
}

void tst_QListWidget::takeItem_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<bool>("outOfBounds");

    QTest::newRow("First item, row: 0") << 0 << false;
    QTest::newRow("Middle item, row: 1") << 1 << false;
    QTest::newRow("Last item, row: 2") << 2 << false;
    QTest::newRow("Out of bounds, row: -1") << -1 << true;
    QTest::newRow("Out of bounds, row: 3") << 3 << true;
}

void tst_QListWidget::takeItem()
{
    QFETCH(int, row);
    QFETCH(bool, outOfBounds);

    (new QListWidgetItem(testWidget))->setText("item0");
    (new QListWidgetItem(testWidget))->setText("item1");
    (new QListWidgetItem(testWidget))->setText("item2");

    QCOMPARE(testWidget->count(), 3);

    QListWidgetItem *item = testWidget->takeItem(row);
    if (outOfBounds) {
        QCOMPARE(item, static_cast<QListWidgetItem*>(0));
        QCOMPARE(testWidget->count(), 3);
    } else {
        QCOMPARE(item->text(), QString("item%1").arg(row));
        QCOMPARE(testWidget->count(), 2);
    }

    delete item;
}

void tst_QListWidget::setItemHidden()
{
    QListWidgetItem *item1 = new QListWidgetItem(testWidget);
    QListWidgetItem *item2 = new QListWidgetItem(testWidget);

    QCOMPARE(testWidget->count(), 2);

    QVERIFY(!testWidget->isItemHidden(item1));
    QVERIFY(!testWidget->isItemHidden(item2));

    testWidget->setItemHidden(item1, true);

    QVERIFY(testWidget->isItemHidden(item1));
    QVERIFY(!testWidget->isItemHidden(item2));
}

void tst_QListWidget::selectedItems_data()
{
    QTest::addColumn<int>("itemCount");
    QTest::addColumn<IntList>("hiddenRows");
    QTest::addColumn<IntList>("selectedRows");
    QTest::addColumn<IntList>("expectedRows");


    QTest::newRow("none hidden, none selected")
        << 3
        << IntList()
        << IntList()
        << IntList();

    QTest::newRow("none hidden, all selected")
        << 3
        << IntList()
        << (IntList() << 0 << 1 << 2)
        << (IntList() << 0 << 1 << 2);

    QTest::newRow("first hidden, all selected")
        << 3
        << (IntList() << 0)
        << (IntList() << 0 << 1 << 2)
        << (IntList() << 0 << 1 << 2);

    QTest::newRow("last hidden, all selected")
        << 3
        << (IntList() << 2)
        << (IntList() << 0 << 1 << 2)
        << (IntList() << 0 << 1 << 2);

    QTest::newRow("middle hidden, all selected")
        << 3
        << (IntList() << 1)
        << (IntList() << 0 << 1 << 2)
        << (IntList() << 0 << 1 << 2);

    QTest::newRow("all hidden, all selected")
        << 3
        << (IntList() << 0 << 1 << 2)
        << (IntList() << 0 << 1 << 2)
        << (IntList() << 0 << 1 << 2);
}

void tst_QListWidget::selectedItems()
{
    QFETCH(int, itemCount);
    QFETCH(IntList, hiddenRows);
    QFETCH(IntList, selectedRows);
    QFETCH(IntList, expectedRows);

    QVERIFY(testWidget->count() == 0);

    //insert items
    for (int i=0; i<itemCount; ++i)
        new QListWidgetItem(QString("Item%1").arg(i), testWidget);

    //verify items are inserted
    QCOMPARE(testWidget->count(), itemCount);
    // hide items
    foreach (int row, hiddenRows)
        testWidget->setItemHidden(testWidget->item(row), true);
    // select items
    foreach (int row, selectedRows)
        testWidget->setItemSelected(testWidget->item(row), true);

    // check that the correct number of items and the expected items are there
    QList<QListWidgetItem *> selectedItems = testWidget->selectedItems();
    QCOMPARE(selectedItems.count(), expectedRows.count());
    foreach (int row, expectedRows)
        QVERIFY(selectedItems.contains(testWidget->item(row)));

    //check that isSelected agrees with selectedItems
    for (int i=0; i<itemCount; ++i) {
        QListWidgetItem *item = testWidget->item(i);
        if (testWidget->isItemSelected(item))
            QVERIFY(selectedItems.contains(item));
    }
}

void tst_QListWidget::insertItem_data()
{
    QTest::addColumn<QStringList>("initialItems");
    QTest::addColumn<int>("insertIndex");
    QTest::addColumn<QString>("itemLabel");
    QTest::addColumn<int>("expectedIndex");

    QStringList initialItems;
    initialItems << "foo" << "bar";

    QTest::newRow("Insert less then 0") << initialItems << -1 << "inserted" << 0;
    QTest::newRow("Insert at 0") << initialItems << 0 << "inserted" << 0;
    QTest::newRow("Insert beyond count") << initialItems << initialItems.count()+1 << "inserted" << initialItems.count();
    QTest::newRow("Insert at count") << initialItems << initialItems.count() << "inserted" << initialItems.count();
    QTest::newRow("Insert in the middle") << initialItems << 1 << "inserted" << 1;
}

void tst_QListWidget::insertItem()
{
    QFETCH(QStringList, initialItems);
    QFETCH(int, insertIndex);
    QFETCH(QString, itemLabel);
    QFETCH(int, expectedIndex);

    testWidget->insertItems(0, initialItems);
    QCOMPARE(testWidget->count(), initialItems.count());

    testWidget->insertItem(insertIndex, itemLabel);

    QCOMPARE(rcFirst[RowsAboutToBeInserted], expectedIndex);
    QCOMPARE(rcLast[RowsAboutToBeInserted], expectedIndex);
    QCOMPARE(rcFirst[RowsInserted], expectedIndex);
    QCOMPARE(rcLast[RowsInserted], expectedIndex);

    QCOMPARE(testWidget->count(), initialItems.count() + 1);
    QCOMPARE(testWidget->item(expectedIndex)->text(), itemLabel);
}

void tst_QListWidget::insertItems_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("insertType");

    QTest::newRow("Insert 1 item using constructor") << 1 << 0;
    QTest::newRow("Insert 10 items using constructor") << 10 << 0;
    QTest::newRow("Insert 100 items using constructor") << 100 << 0;

    QTest::newRow("Insert 1 item with insertItem") << 1 << 1;
    QTest::newRow("Insert 10 items with insertItem") << 10 << 1;
    QTest::newRow("Insert 100 items with insertItem") << 100 << 1;

    QTest::newRow("Insert/Create 1 item using insertItem") << 1 << 2;
    QTest::newRow("Insert/Create 10 items using insertItem") << 10 << 2;
    QTest::newRow("Insert/Create 100 items using insertItem") << 100 << 2;

    QTest::newRow("Insert 1 item with insertItems") << 1 << 3;
    QTest::newRow("Insert 10 items with insertItems") << 10 << 3;
    QTest::newRow("Insert 100 items with insertItems") << 100 << 3;
}

void tst_QListWidget::insertItems()
{
    QFETCH(int, rowCount);
    QFETCH(int, insertType);

    for (int r = 0; r < rowCount; ++r) {
        if (insertType == 0) {
            // insert with QListWidgetItem constructor
            new QListWidgetItem(QString::number(r), testWidget);
        } else if (insertType == 1) {
            // insert actual item
            testWidget->insertItem(r, new QListWidgetItem(QString::number(r)));
        } else if (insertType == 2) {
            // insert/creating with string
            testWidget->insertItem(r, QString::number(r));
        } else if (insertType == 3) {
            QStringList strings;
            for (int i=0; i<rowCount; ++i)
                strings << QString::number(i);
            testWidget->insertItems(0, strings);
            break;
        } else {
            QVERIFY(0);
        }
    }

    // compare the results
    QCOMPARE(testWidget->count(), rowCount);

    // check if the text
    for (int r = 0; r < rowCount; ++r)
        QCOMPARE(testWidget->item(r)->text(), QString::number(r));

    // make sure all items have view set correctly
    for (int i=0; i<testWidget->count(); ++i)
        QCOMPARE(testWidget->item(i)->listWidget(), testWidget);
}

void tst_QListWidget::removeItems_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("removeRows");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("expectedRowCount");

    QTest::newRow("Empty") << 0 << 1 << 0 << 0;
    QTest::newRow("1:1") << 1 << 1 << 0 << 0;
    QTest::newRow("3:1") << 3 << 1 << 0 << 2;
    QTest::newRow("3:2") << 3 << 2 << 0 << 1;
    QTest::newRow("100:10") << 100 << 10 << 0 << 90;
}

void tst_QListWidget::removeItems()
{
    QFETCH(int, rowCount);
    QFETCH(int, removeRows);
    QFETCH(int, row);
    QFETCH(int, expectedRowCount);

    //insert items
    for (int r = 0; r < rowCount; ++r)
        new QListWidgetItem(QString::number(r), testWidget);

    // remove and compare the results
    for (int r = 0; r < removeRows; ++r)
        delete testWidget->item(row);
    QCOMPARE(testWidget->count(), expectedRowCount);

    // check if the correct items were removed
    for (int r = 0; r < expectedRowCount; ++r)
        if (r < row)
            QCOMPARE(testWidget->item(r)->text(), QString::number(r));
        else
            QCOMPARE(testWidget->item(r)->text(), QString::number(r + removeRows));


}

void tst_QListWidget::itemStreaming_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("toolTip");

    QTest::newRow("Data") << "item text" << "tool tip text";
}

void tst_QListWidget::itemStreaming()
{
    QFETCH(QString, text);
    QFETCH(QString, toolTip);

    QListWidgetItem item;
    QCOMPARE(item.text(), QString());
    QCOMPARE(item.toolTip(), QString());

    item.setText(text);
    item.setToolTip(toolTip);
    QCOMPARE(item.text(), text);
    QCOMPARE(item.toolTip(), toolTip);

    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << item;

    QListWidgetItem item2;
    QCOMPARE(item2.text(), QString());
    QCOMPARE(item2.toolTip(), QString());

    QVERIFY(!buffer.isEmpty());

    QDataStream in(&buffer, QIODevice::ReadOnly);
    in >> item2;
    QCOMPARE(item2.text(), text);
    QCOMPARE(item2.toolTip(), toolTip);
}

void tst_QListWidget::sortItems_data()
{
    QTest::addColumn<int>("order");
    QTest::addColumn<QStringList>("initialList");
    QTest::addColumn<QStringList>("expectedList");
    QTest::addColumn<IntList>("expectedRows");

    QTest::newRow("ascending order")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "c" << "d" << "a" << "b")
        << (QStringList() << "a" << "b" << "c" << "d")
        << (IntList() << 2 << 3 << 0 << 1);

    QTest::newRow("descending order")
        << static_cast<int>(Qt::DescendingOrder)
        << (QStringList() << "c" << "d" << "a" << "b")
        << (QStringList() << "d" << "c" << "b" << "a")
        << (IntList() << 1 << 0 << 3 << 2);
}

void tst_QListWidget::sortItems()
{
    QFETCH(int, order);
    QFETCH(QStringList, initialList);
    QFETCH(QStringList, expectedList);
    QFETCH(IntList, expectedRows);

    testWidget->addItems(initialList);

    QAbstractItemModel *model = testWidget->model();
    QList<QPersistentModelIndex> persistent;
    for (int j = 0; j < model->rowCount(QModelIndex()); ++j)
        persistent << model->index(j, 0, QModelIndex());

    testWidget->sortItems(static_cast<Qt::SortOrder>(order));

    QCOMPARE(testWidget->count(), expectedList.count());
    for (int i = 0; i < testWidget->count(); ++i)
        QCOMPARE(testWidget->item(i)->text(), expectedList.at(i));

    for (int k = 0; k < testWidget->count(); ++k)
        QCOMPARE(persistent.at(k).row(), expectedRows.at(k));
}

void tst_QListWidget::modelChanged(ModelChanged change, const QModelIndex &parent,
                                   int first, int last)
{
    rcParent[change] = parent;
    rcFirst[change] = first;
    rcLast[change] = last;
}

class TestListWidget : public QListWidget {
public:
    TestListWidget() : QListWidget()
    {

    }
    State getState() {return QListWidget::state();}

    void closeEditor(QWidget *w, QAbstractItemDelegate::EndEditHint hint) {
        QListWidget::closeEditor(w, hint);
    }

    bool isEditingState(QListWidgetItem *item) {
        Q_UNUSED(item);
        return (QListWidget::state() == QListWidget::EditingState ? true : false);
    }
};

void tst_QListWidget::closeEditor()
{
    TestListWidget w;
    QStringList labels = (QStringList() << "a" << "b" << "c" << "d");
    w.addItems(labels);
    QListWidgetItem *item = w.item(0);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    QVERIFY(item);
    w.editItem(item);

    QVERIFY(w.isEditingState(item));

    w.reset();

    QVERIFY(!w.isEditingState(item));
}

void tst_QListWidget::setData_data()
{
    QTest::addColumn<QStringList>("initialItems");
    QTest::addColumn<int>("itemIndex");
    QTest::addColumn<IntList>("roles");
    QTest::addColumn<QVariantList>("values");
    QTest::addColumn<int>("expectedSignalCount");

    QStringList initialItems;
    IntList roles;
    QVariantList values;

    {
        initialItems.clear(); roles.clear(); values.clear();
        initialItems << "foo";
        roles << Qt::DisplayRole;
        values << "xxx";
        QTest::newRow("changing a role should emit")
            << initialItems << 0 << roles << values << 1;
    }
    {
        initialItems.clear(); roles.clear(); values.clear();
        initialItems << "foo";
        roles << Qt::DisplayRole;
        values << "foo";
        QTest::newRow("setting the same value should not emit")
            << initialItems << 0 << roles << values << 0;
    }
    {
        initialItems.clear(); roles.clear(); values.clear();
        initialItems << "foo";
        roles << Qt::DisplayRole << Qt::DisplayRole;
        values << "bar" << "bar";
        QTest::newRow("setting the same value twice should only emit once")
            << initialItems << 0 << roles << values << 1;
    }
    {
        initialItems.clear(); roles.clear(); values.clear();
        initialItems << "foo";
        roles << Qt::DisplayRole << Qt::ToolTipRole << Qt::WhatsThisRole;
        values << "bar" << "bartooltip" << "barwhatsthis";
        QTest::newRow("changing three roles should emit three times")
            << initialItems << 0 << roles << values << 3;
    }
}

void tst_QListWidget::setData()
{
    QFETCH(QStringList, initialItems);
    QFETCH(int, itemIndex);
    QFETCH(IntList, roles);
    QFETCH(QVariantList, values);
    QFETCH(int, expectedSignalCount);
    qRegisterMetaType<QListWidgetItem *>("QListWidgetItem*");
    qRegisterMetaType<QModelIndex>("QModelIndex");

    QVERIFY(roles.count() == values.count());

    for (int manipulateModel=0; manipulateModel<2; ++manipulateModel) {
        testWidget->clear();
        testWidget->insertItems(0, initialItems);
        QCOMPARE(testWidget->count(), initialItems.count());

        QSignalSpy itemChanged(testWidget, SIGNAL(itemChanged(QListWidgetItem *)));
        QSignalSpy dataChanged(testWidget->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)));

        for (int i=0; i < roles.count(); ++i) {
            if (manipulateModel)
                testWidget->model()->setData(
                    testWidget->model()->index(itemIndex, 0, testWidget->rootIndex()),
                    values.at(i),
                    roles.at(i));
            else
                testWidget->item(itemIndex)->setData(roles.at(i), values.at(i));
        }

        // make sure the data is actually set
        for (int i=0; i < roles.count(); ++i)
            QCOMPARE(testWidget->item(itemIndex)->data(roles.at(i)), values.at(i));

        // make sure we get the right number of emits
        QCOMPARE(itemChanged.count(), expectedSignalCount);
        QCOMPARE(dataChanged.count(), expectedSignalCount);
    }
}

#if QT_VERSION >= 0x040200
void tst_QListWidget::insertItemsWithSorting_data()
{
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<QStringList>("initialItems");
    QTest::addColumn<QStringList>("insertItems");
    QTest::addColumn<QStringList>("expectedItems");
    QTest::addColumn<IntList>("expectedRows");

    QTest::newRow("() + (a) = (a)")
        << static_cast<int>(Qt::AscendingOrder)
        << QStringList()
        << (QStringList() << "a")
        << (QStringList() << "a")
        << IntList();
    QTest::newRow("() + (c, b, a) = (a, b, c)")
        << static_cast<int>(Qt::AscendingOrder)
        << QStringList()
        << (QStringList() << "c" << "b" << "a")
        << (QStringList() << "a" << "b" << "c")
        << IntList();
    QTest::newRow("() + (a, b, c) = (c, b, a)")
        << static_cast<int>(Qt::DescendingOrder)
        << QStringList()
        << (QStringList() << "a" << "b" << "c")
        << (QStringList() << "c" << "b" << "a")
        << IntList();
    QTest::newRow("(a) + (b) = (a, b)")
        << static_cast<int>(Qt::AscendingOrder)
        << QStringList("a")
        << (QStringList() << "b")
        << (QStringList() << "a" << "b")
        << (IntList() << 0);
    QTest::newRow("(a) + (b) = (b, a)")
        << static_cast<int>(Qt::DescendingOrder)
        << QStringList("a")
        << (QStringList() << "b")
        << (QStringList() << "b" << "a")
        << (IntList() << 1);
    QTest::newRow("(a, c, b) + (d) = (a, b, c, d)")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "a" << "c" << "b")
        << (QStringList() << "d")
        << (QStringList() << "a" << "b" << "c" << "d")
        << (IntList() << 0 << 1 << 2);
    QTest::newRow("(b, c, a) + (d) = (d, c, b, a)")
        << static_cast<int>(Qt::DescendingOrder)
        << (QStringList() << "b" << "c" << "a")
        << (QStringList() << "d")
        << (QStringList() << "d" << "c" << "b" << "a")
        << (IntList() << 1 << 2 << 3);
    {
        IntList ascendingRows;
        IntList reverseRows;
        QStringList ascendingItems;
        QStringList reverseItems;
        for (int i = 'a'; i <= 'z'; ++i) {
            ascendingItems << QString("%0").arg(QLatin1Char(i));
            reverseItems << QString("%0").arg(QLatin1Char('z' - i + 'a'));
            ascendingRows << i - 'a';
            reverseRows << 'z' - i + 'a';
        }
        QTest::newRow("() + (sorted items) = (sorted items)")
            << static_cast<int>(Qt::AscendingOrder)
            << QStringList()
            << ascendingItems
            << ascendingItems
            << IntList();
        QTest::newRow("(sorted items) + () = (sorted items)")
            << static_cast<int>(Qt::AscendingOrder)
            << ascendingItems
            << QStringList()
            << ascendingItems
            << ascendingRows;
        QTest::newRow("() + (ascending items) = (reverse items)")
            << static_cast<int>(Qt::DescendingOrder)
            << QStringList()
            << ascendingItems
            << reverseItems
            << IntList();
        QTest::newRow("(reverse items) + () = (ascending items)")
            << static_cast<int>(Qt::AscendingOrder)
            << reverseItems
            << QStringList()
            << ascendingItems
            << ascendingRows;
        QTest::newRow("(reverse items) + () = (reverse items)")
            << static_cast<int>(Qt::DescendingOrder)
            << reverseItems
            << QStringList()
            << reverseItems
            << ascendingRows;
    }
}

void tst_QListWidget::insertItemsWithSorting()
{
    QFETCH(int, sortOrder);
    QFETCH(QStringList, initialItems);
    QFETCH(QStringList, insertItems);
    QFETCH(QStringList, expectedItems);
    QFETCH(IntList, expectedRows);

    for (int method = 0; method < 5; ++method) {
        QListWidget w;
        w.setSortingEnabled(true);
        w.sortItems(static_cast<Qt::SortOrder>(sortOrder));
        w.addItems(initialItems);

        QAbstractItemModel *model = w.model();
        QList<QPersistentModelIndex> persistent;
        for (int j = 0; j < model->rowCount(QModelIndex()); ++j)
            persistent << model->index(j, 0, QModelIndex());

        switch (method) {
            case 0:
                // insert using item constructor
                for (int i = 0; i < insertItems.size(); ++i)
                    new QListWidgetItem(insertItems.at(i), &w);
                break;
            case 1:
                // insert using insertItems()
                w.insertItems(0, insertItems);
                break;
            case 2:
                // insert using insertItem()
                for (int i = 0; i < insertItems.size(); ++i)
                    w.insertItem(0, insertItems.at(i));
                break;
            case 3:
                // insert using addItems()
                w.addItems(insertItems);
                break;
            case 4:
                // insert using addItem()
                for (int i = 0; i < insertItems.size(); ++i)
                    w.addItem(insertItems.at(i));
                break;
        }
        QCOMPARE(w.count(), expectedItems.count());
        for (int i = 0; i < w.count(); ++i)
            QCOMPARE(w.item(i)->text(), expectedItems.at(i));

        for (int k = 0; k < persistent.count(); ++k)
            QCOMPARE(persistent.at(k).row(), expectedRows.at(k));
    }
}

void tst_QListWidget::changeDataWithSorting_data()
{
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<QStringList>("initialItems");
    QTest::addColumn<int>("itemIndex");
    QTest::addColumn<QString>("newValue");
    QTest::addColumn<QStringList>("expectedItems");
    QTest::addColumn<IntList>("expectedRows");
    QTest::addColumn<bool>("reorderingExpected");

    QTest::newRow("change a to b in (a)")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "a")
        << 0 << "b"
        << (QStringList() << "b")
        << (IntList() << 0)
        << false;
    QTest::newRow("change a to b in (a, c)")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "a" << "c")
        << 0 << "b"
        << (QStringList() << "b" << "c")
        << (IntList() << 0 << 1)
        << false;
    QTest::newRow("change a to c in (a, b)")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "a" << "b")
        << 0 << "c"
        << (QStringList() << "b" << "c")
        << (IntList() << 1 << 0)
        << true;
    QTest::newRow("change c to a in (c, b)")
        << static_cast<int>(Qt::DescendingOrder)
        << (QStringList() << "c" << "b")
        << 0 << "a"
        << (QStringList() << "b" << "a")
        << (IntList() << 1 << 0)
        << true;
    QTest::newRow("change e to i in (a, c, e, g)")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "a" << "c" << "e" << "g")
        << 2 << "i"
        << (QStringList() << "a" << "c" << "g" << "i")
        << (IntList() << 0 << 1 << 3 << 2)
        << true;
    QTest::newRow("change e to a in (c, e, g, i)")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "c" << "e" << "g" << "i")
        << 1 << "a"
        << (QStringList() << "a" << "c" << "g" << "i")
        << (IntList() << 1 << 0 << 2 << 3)
        << true;
    QTest::newRow("change e to f in (c, e, g, i)")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "c" << "e" << "g" << "i")
        << 1 << "f"
        << (QStringList() << "c" << "f" << "g" << "i")
        << (IntList() << 0 << 1 << 2 << 3)
        << false;
}

void tst_QListWidget::changeDataWithSorting()
{
    QFETCH(int, sortOrder);
    QFETCH(QStringList, initialItems);
    QFETCH(int, itemIndex);
    QFETCH(QString, newValue);
    QFETCH(QStringList, expectedItems);
    QFETCH(IntList, expectedRows);
    QFETCH(bool, reorderingExpected);

    QListWidget w;
    w.setSortingEnabled(true);
    w.sortItems(static_cast<Qt::SortOrder>(sortOrder));
    w.addItems(initialItems);

    QAbstractItemModel *model = w.model();
    QList<QPersistentModelIndex> persistent;
    for (int j = 0; j < model->rowCount(QModelIndex()); ++j)
        persistent << model->index(j, 0, QModelIndex());

    QSignalSpy dataChangedSpy(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)));
    QSignalSpy layoutChangedSpy(model, SIGNAL(layoutChanged()));

    QListWidgetItem *item = w.item(itemIndex);
    item->setText(newValue);
    for (int i = 0; i < expectedItems.count(); ++i)
        QCOMPARE(w.item(i)->text(), expectedItems.at(i));

    for (int k = 0; k < persistent.count(); ++k)
        QCOMPARE(persistent.at(k).row(), expectedRows.at(k));

    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(layoutChangedSpy.count(), reorderingExpected ? 1 : 0);
}
#endif // QT_VERSION

QTEST_MAIN(tst_QListWidget)
#include "tst_qlistwidget.moc"
