/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qtreewidget.h>
#include <qtreewidgetitemiterator.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <qdebug.h>
#include <qheaderview.h>
#include <qlineedit.h>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qtreewidget.h gui/itemviews/qtreewidget.cpp

class tst_QTreeWidget : public QObject
{
    Q_OBJECT

public:
    tst_QTreeWidget();
    ~tst_QTreeWidget();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void getSetCheck();
    void addTopLevelItem();
    void currentItem_data();
    void currentItem();
    void editItem_data();
    void editItem();
    void takeItem_data();
    void takeItem();
    void setItemHidden();
    void setItemHidden2();
    void selectedItems_data();
    void selectedItems();
    void itemAssignment();
    void clone_data();
    void clone();
    void expand_data();
    void expand();
    void checkState_data();
    void checkState();
    void findItems_data();
    void findItems();
    void findItemsInColumn();
    void sortItems_data();
    void sortItems();
    void deleteItems_data();
    void deleteItems();
    void itemStreaming_data();
    void itemStreaming();
    void insertTopLevelItems_data();
    void insertTopLevelItems();
    void keyboardNavigation();
    void scrollToItem();
    void setSortingEnabled();
    void match();
    void columnCount();
    void setHeaderLabels();
    void setHeaderItem();
    void itemWidget_data();
    void itemWidget();
    void insertItemsWithSorting_data();
    void insertItemsWithSorting();
    void changeDataWithSorting_data();
    void changeDataWithSorting();

    // QTreeWidgetItem
    void itemOperatorLessThan();
    void addChild();
    void setData();

private:
    QTreeWidget *testWidget;
};

// Testing get/set functions
void tst_QTreeWidget::getSetCheck()
{
    QTreeWidget obj1;
    // int QTreeWidget::columnCount()
    // void QTreeWidget::setColumnCount(int)
    obj1.setColumnCount(0);
    QCOMPARE(obj1.columnCount(), 0);

    obj1.setColumnCount(INT_MIN);
    QCOMPARE(obj1.columnCount(), 0);

    //obj1.setColumnCount(INT_MAX);
    //QCOMPARE(obj1.columnCount(), INT_MAX);
    // Since setColumnCount allocates memory, there is no way this will succeed

    obj1.setColumnCount(100);
    QCOMPARE(obj1.columnCount(), 100);

    // QTreeWidgetItem * QTreeWidget::headerItem()
    // void QTreeWidget::setHeaderItem(QTreeWidgetItem *)
    QTreeWidgetItem *var2 = new QTreeWidgetItem();
    obj1.setHeaderItem(var2);
    QCOMPARE(obj1.headerItem(), var2);

    obj1.setHeaderItem((QTreeWidgetItem *)0);
//    QCOMPARE(obj1.headerItem(), (QTreeWidgetItem *)0);

    // QTreeWidgetItem * QTreeWidget::currentItem()
    // void QTreeWidget::setCurrentItem(QTreeWidgetItem *)
    QTreeWidgetItem *var3 = new QTreeWidgetItem(&obj1);
    obj1.setCurrentItem(var3);
    QCOMPARE(obj1.currentItem(), var3);

    obj1.setCurrentItem((QTreeWidgetItem *)0);
    QCOMPARE(obj1.currentItem(), (QTreeWidgetItem *)0);
}

typedef QList<int> IntList;
typedef QList<IntList> ListIntList;

Q_DECLARE_METATYPE(IntList)
Q_DECLARE_METATYPE(ListIntList)
Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(Qt::Orientation)

typedef QTreeWidgetItem TreeItem;
typedef QList<TreeItem*> TreeItemList;

Q_DECLARE_METATYPE(QTreeWidgetItem*)
Q_DECLARE_METATYPE(TreeItemList)

tst_QTreeWidget::tst_QTreeWidget(): testWidget(0)
{
}

tst_QTreeWidget::~tst_QTreeWidget()
{
}

void tst_QTreeWidget::initTestCase()
{
    qMetaTypeId<QModelIndex>();
    qMetaTypeId<Qt::Orientation>();
    qRegisterMetaType<QTreeWidgetItem*>("QTreeWidgetItem*");

    testWidget = new QTreeWidget();
    testWidget->show();
}

void tst_QTreeWidget::cleanupTestCase()
{
    testWidget->hide();
    delete testWidget;
}

void tst_QTreeWidget::init()
{
    testWidget->clear();
    testWidget->setColumnCount(2);
}

void tst_QTreeWidget::cleanup()
{
}

TreeItem *operator<<(TreeItem *parent, const TreeItemList &children) {
    for (int i = 0; i < children.count(); ++i)
        parent->addChild(children.at(i));
    return parent;
}

static void populate(QTreeWidget *widget, const TreeItemList &topLevelItems,
                     TreeItem *headerItem = 0)
{
    widget->clear();
    widget->setHeaderItem(headerItem);
    foreach (TreeItem *item, topLevelItems)
        widget->addTopLevelItem(item);
}

void tst_QTreeWidget::addTopLevelItem()
{
    QTreeWidget tree;
    QCOMPARE(tree.topLevelItemCount(), 0);

    // try to add 0
    tree.addTopLevelItem(0);
    QCOMPARE(tree.topLevelItemCount(), 0);
    QCOMPARE(tree.indexOfTopLevelItem(0), -1);

    // add one at a time
    QList<TreeItem*> tops;
    for (int i = 0; i < 10; ++i) {
        TreeItem *ti = new TreeItem();
        QCOMPARE(tree.indexOfTopLevelItem(ti), -1);
        tree.addTopLevelItem(ti);
        QCOMPARE(tree.topLevelItemCount(), i+1);
        QCOMPARE(tree.topLevelItem(i), ti);
        QCOMPARE(tree.topLevelItem(-1), static_cast<TreeItem*>(0));
        QCOMPARE(tree.indexOfTopLevelItem(ti), i);
        QCOMPARE(ti->parent(), static_cast<TreeItem*>(0));
        tree.addTopLevelItem(ti);
        QCOMPARE(tree.topLevelItemCount(), i+1);
        tops.append(ti);
    }

    // delete one at a time
    while (!tops.isEmpty()) {
        TreeItem *ti = tops.takeFirst();
        delete ti;
        QCOMPARE(tree.topLevelItemCount(), tops.count());
        for (int i = 0; i < tops.count(); ++i)
            QCOMPARE(tree.topLevelItem(i), tops.at(i));
    }

    // add none
    {
        int count = tree.topLevelItemCount();
        tree.addTopLevelItems(tops);
        QCOMPARE(tree.topLevelItemCount(), count);
    }

    // add many at a time
    {
        const int count = 10;
        for (int i = 0; i < 100; i += count) {
            tops.clear();
            for (int j = 0; j < count; ++j)
                tops << new TreeItem(QStringList() << QString("%0").arg(j));
            tree.addTopLevelItems(tops);
            QCOMPARE(tree.topLevelItemCount(), count + i);
            for (int j = 0; j < count; ++j) {
                // ### items are added in reverse order (task 118101)
                QCOMPARE(tree.topLevelItem(i+j), tops.at(tops.count() - 1 - j));
            }
            
            tree.addTopLevelItems(tops);
            QCOMPARE(tree.topLevelItemCount(), count + i);
        }
    }

    // insert
    {
        tops.clear();
        for (int i = 0; i < 10; ++i)
            tops << new TreeItem();
        int count = tree.topLevelItemCount();
        tree.insertTopLevelItems(100000, tops);
        // ### fixme
        QCOMPARE(tree.topLevelItemCount(), count + 10);
    }
}

void tst_QTreeWidget::currentItem_data()
{
    QTest::addColumn<TreeItemList>("topLevelItems");

    QTest::newRow("only top-level items, 2 columns")
        << (TreeItemList()
            << new TreeItem(QStringList() << "a" << "b")
            << new TreeItem(QStringList() << "c" << "d"));
    QTest::newRow("hierarchy, 2 columns")
        << (TreeItemList()
            << (new TreeItem(QStringList() << "a" << "b")
                << (TreeItemList()
                    << new TreeItem(QStringList() << "c" << "d")
                    << new TreeItem(QStringList() << "c" << "d")
                    )
                )
            << (new TreeItem(QStringList() << "e" << "f")
                << (TreeItemList()
                    << new TreeItem(QStringList() << "g" << "h")
                    << new TreeItem(QStringList() << "g" << "h")
                    )
                ));
}

void tst_QTreeWidget::currentItem()
{
    QFETCH(TreeItemList, topLevelItems);

    QTreeWidget tree;
    tree.show();
    populate(&tree, topLevelItems, new TreeItem(QStringList() << "1" << "2"));
    QTreeWidgetItem *previous = 0;
    for (int x = 0; x < 2; ++x) {
        tree.setSelectionBehavior(x ? QAbstractItemView::SelectItems
                                  : QAbstractItemView::SelectRows);
        QSignalSpy currentItemChangedSpy(
            &tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
        QSignalSpy itemSelectionChangedSpy(
            &tree, SIGNAL(itemSelectionChanged()));
        
        QTreeWidgetItemIterator it(&tree);
        // do all items
        while (QTreeWidgetItem *item = (*it++)) {
            tree.setCurrentItem(item);
            QCOMPARE(tree.currentItem(), item);

            QCOMPARE(currentItemChangedSpy.count(), 1);
            QVariantList args = currentItemChangedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
            QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(1)), previous);

            QCOMPARE(itemSelectionChangedSpy.count(), 1);
            itemSelectionChangedSpy.clear();

            previous = item;
            // do all columns
            for (int col = 0; col < item->columnCount(); ++col) {
                tree.setCurrentItem(item, col);
                QCOMPARE(tree.currentItem(), item);
                QCOMPARE(tree.currentColumn(), col);

                if (!currentItemChangedSpy.isEmpty()) {
                    // ### we get a currentItemChanged() when what really
                    // changed was just currentColumn(). Should it be like this?
                    QCOMPARE(currentItemChangedSpy.count(), 1);
                    QVariantList args = currentItemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(1)), item);
                    if (tree.selectionBehavior() == QAbstractItemView::SelectItems) {
                        QCOMPARE(itemSelectionChangedSpy.count(), 1);
                        itemSelectionChangedSpy.clear();
                    } else {
                        QCOMPARE(itemSelectionChangedSpy.count(), 0);
                    }
                }
            }
        }
    }

    // can't set the headerItem to be the current item
    tree.setCurrentItem(tree.headerItem());
    QCOMPARE(tree.currentItem(), static_cast<TreeItem*>(0));
}

void tst_QTreeWidget::editItem_data()
{
    QTest::addColumn<TreeItemList>("topLevelItems");

    {
        TreeItemList list;
        for (int i = 0; i < 10; i++) {
            TreeItem *item = new TreeItem(QStringList() << "" << "");
            if ((i & 1) == 0)
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            else
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            list << item;
        }
        QTest::newRow("2 columns, only even items editable")
            << list;
    }
}

void tst_QTreeWidget::editItem()
{
    QFETCH(TreeItemList, topLevelItems);

    QTreeWidget tree;
    populate(&tree, topLevelItems, new TreeItem(QStringList() << "1" << "2"));
    tree.show();

    QSignalSpy itemChangedSpy(
        &tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)));

    QTreeWidgetItemIterator it(&tree);
    while (QTreeWidgetItem *item = (*it++)) {
        for (int col = 0; col < item->columnCount(); ++col) {
            if (!(item->flags() & Qt::ItemIsEditable))
                QTest::ignoreMessage(QtWarningMsg, "edit: editing failed");
            tree.editItem(item, col);
            QApplication::instance()->processEvents();
            QWidget *editor = tree.findChild<QLineEdit*>();
            if (editor) {
                QVERIFY(item->flags() & Qt::ItemIsEditable);
                QTest::keyClick(editor, Qt::Key_A);
                QTest::keyClick(editor, Qt::Key_Enter);
                QApplication::instance()->processEvents();
                delete editor;
                QCOMPARE(itemChangedSpy.count(), 1);
                QVariantList args = itemChangedSpy.takeFirst();
                QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                QCOMPARE(qvariant_cast<int>(args.at(1)), col);
            } else {
                QVERIFY(!(item->flags() & Qt::ItemIsEditable));
            }
        }
    }
}

void tst_QTreeWidget::takeItem_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<bool>("topLevel");
    QTest::addColumn<bool>("outOfBounds");

    QTest::newRow("First, topLevel") << 0 << true << false;
    QTest::newRow("Last, topLevel") << 2 << true << false;
    QTest::newRow("Middle, topLevel") << 1 << true << false;
    QTest::newRow("Out of bounds, toplevel, (index: -1)") << -1 << true << true;
    QTest::newRow("Out of bounds, toplevel, (index: 3)") << 3 << true << true;

    QTest::newRow("First, child of topLevel") << 0 << false << false;
    QTest::newRow("Last, child of topLevel") << 2 << false << false;
    QTest::newRow("Middle, child of topLevel") << 1 << false << false;
    QTest::newRow("Out of bounds, child of toplevel, (index: -1)") << -1 << false << true;
    QTest::newRow("Out of bounds, child of toplevel, (index: 3)") << 3 << false << true;
}

void tst_QTreeWidget::takeItem()
{
    QFETCH(int, index);
    QFETCH(bool, topLevel);
    QFETCH(bool, outOfBounds);

    for (int i=0; i<3; ++i) {
        QTreeWidgetItem *top = new QTreeWidgetItem(testWidget);
        top->setText(0, QString("top%1").arg(i));
        for (int j=0; j<3; ++j) {
            QTreeWidgetItem *child = new QTreeWidgetItem(top);
            child->setText(0, QString("child%1").arg(j));
        }
    }

    QCOMPARE(testWidget->topLevelItemCount(), 3);
    QCOMPARE(testWidget->topLevelItem(0)->childCount(), 3);

    if (topLevel) {
        int count = testWidget->topLevelItemCount();
        QTreeWidgetItem *item = testWidget->takeTopLevelItem(index);
        if (outOfBounds) {
            QCOMPARE(item, (QTreeWidgetItem *)0);
            QCOMPARE(count, testWidget->topLevelItemCount());
        } else {
            QCOMPARE(item->text(0), QString("top%1").arg(index));
            QCOMPARE(count-1, testWidget->topLevelItemCount());
            delete item;
        }
    } else {
        int count = testWidget->topLevelItem(0)->childCount();
        QTreeWidgetItem *item = testWidget->topLevelItem(0)->takeChild(index);
        if (outOfBounds) {
            QCOMPARE(item, (QTreeWidgetItem *)0);
            QCOMPARE(count, testWidget->topLevelItem(0)->childCount());
        } else {
            QCOMPARE(item->text(0), QString("child%1").arg(index));
            QCOMPARE(count-1, testWidget->topLevelItem(0)->childCount());
            delete item;
        }
    }
}

void tst_QTreeWidget::setItemHidden()
{
    QTreeWidgetItem *parent = new QTreeWidgetItem(testWidget);
    parent->setText(0, "parent");
    QTreeWidgetItem *child = new QTreeWidgetItem(parent);
    child->setText(0, "child");
    QVERIFY(child->parent());

    testWidget->expandItem(parent);
    testWidget->scrollToItem(child);

    QVERIFY(testWidget->visualItemRect(parent).isValid()
           && testWidget->viewport()->rect().contains(testWidget->visualItemRect(parent)));
    QVERIFY(testWidget->visualItemRect(child).isValid()
           && testWidget->viewport()->rect().contains(testWidget->visualItemRect(child)));

    QVERIFY(!testWidget->isItemHidden(parent));
    QVERIFY(!testWidget->isItemHidden(child));

    testWidget->setItemHidden(parent, true);

    QVERIFY(!(testWidget->visualItemRect(parent).isValid()
             && testWidget->viewport()->rect().contains(testWidget->visualItemRect(parent))));
    QVERIFY(!(testWidget->visualItemRect(child).isValid()
             && testWidget->viewport()->rect().contains(testWidget->visualItemRect(child))));

    QVERIFY(testWidget->isItemHidden(parent));
    QVERIFY(!testWidget->isItemHidden(child));

    // From task 78670 (This caused an core dump)
    // Check if we can set an item visible if it already is visible.
    testWidget->setItemHidden(parent, false);
    testWidget->setItemHidden(parent, false);
    QVERIFY(!testWidget->isItemHidden(parent));


    // hide, hide and then unhide.
    testWidget->setItemHidden(parent, true);
    testWidget->setItemHidden(parent, true);
    testWidget->setItemHidden(parent, false);
    QVERIFY(!testWidget->isItemHidden(parent));


}


void tst_QTreeWidget::setItemHidden2()
{
    // From Task 78587
    QStringList hl;
    hl << "ID" << "Desc";
    testWidget->setColumnCount(hl.count());
    testWidget->setHeaderLabels(hl);
    testWidget->setSortingEnabled(true);

    QTreeWidgetItem *top = new QTreeWidgetItem(testWidget);
    QTreeWidgetItem *leaf = 0;
    top->setText(0, "ItemList");
    for (int i = 1; i <= 4; i++) {
        leaf = new QTreeWidgetItem(top);
        leaf->setText(0, QString().sprintf("%d", i));
        leaf->setText(1, QString().sprintf("Item %d", i));
    }

    if (testWidget->topLevelItemCount() > 0) {
        top = testWidget->topLevelItem(0);
        testWidget->setItemExpanded(top, true);
    }

    if (testWidget->topLevelItemCount() > 0) {
        top = testWidget->topLevelItem(0);
        for (int i = 0; i < top->childCount(); i++) {
            leaf = top->child(i);
            if (leaf->text(0).toInt() % 2 == 0) {
                if (!testWidget->isItemHidden(leaf)) {
                    testWidget->setItemHidden(leaf, true);
                }
            }
        }
    }
}


void tst_QTreeWidget::selectedItems_data()
{
    QTest::addColumn<int>("topLevel");
    QTest::addColumn<int>("children");
    QTest::addColumn<bool>("closeTopLevel");
    QTest::addColumn<ListIntList>("selectedItems");
    QTest::addColumn<ListIntList>("hiddenItems");
    QTest::addColumn<ListIntList>("expectedItems");

    ListIntList selectedItems;
    ListIntList hiddenItems;
    ListIntList expectedItems;

    selectedItems.clear(); hiddenItems.clear(); expectedItems.clear();
    selectedItems << (IntList() << 0);
    expectedItems << (IntList() << 0);
    QTest::newRow("2 top with 2 children, closed, top0 selected, no hidden")
        << 2 << 2 << true << selectedItems << hiddenItems << expectedItems;

    selectedItems.clear(); hiddenItems.clear(); expectedItems.clear();
    selectedItems << (IntList() << 0 << 0);
    expectedItems << (IntList() << 0 << 0);
    QTest::newRow("2 top with 2 children, closed, top0child0 selected, no hidden")
        << 2 << 2 << true << selectedItems << hiddenItems << expectedItems;

    selectedItems.clear(); hiddenItems.clear(); expectedItems.clear();
    selectedItems << (IntList() << 0 << 0);
    expectedItems << (IntList() << 0 << 0);
    QTest::newRow("2 top with 2 children, open, top0child0 selected, no hidden")
        << 2 << 2 << false << selectedItems << hiddenItems << expectedItems;

    selectedItems.clear(); hiddenItems.clear(); expectedItems.clear();
    selectedItems << (IntList() << 0);
    hiddenItems << (IntList() << 0);
#if QT_VERSION >= 0x040100
    expectedItems << (IntList() << 0);
#endif
    QTest::newRow("2 top with 2 children, closed, top0 selected, top0 hidden")
        << 2 << 2 << true << selectedItems << hiddenItems << expectedItems;

    selectedItems.clear(); hiddenItems.clear(); expectedItems.clear();
    selectedItems << (IntList() << 0 << 0);
    hiddenItems << (IntList() << 0);
#if QT_VERSION >= 0x040100
    expectedItems << (IntList() << 0 << 0);
#endif
    QTest::newRow("2 top with 2 children, closed, top0child0 selected, top0 hidden")
        << 2 << 2 << true << selectedItems << hiddenItems << expectedItems;

    selectedItems.clear(); hiddenItems.clear(); expectedItems.clear();
    selectedItems
        << (IntList() << 0)
        << (IntList() << 0 << 0)
        << (IntList() << 0 << 1)
        << (IntList() << 1)
        << (IntList() << 1 << 0)
        << (IntList() << 1 << 1);
    expectedItems
        << (IntList() << 0)
        << (IntList() << 0 << 0)
        << (IntList() << 0 << 1)
        << (IntList() << 1)
        << (IntList() << 1 << 0)
        << (IntList() << 1 << 1);
    QTest::newRow("2 top with 2 children, closed, all selected, no hidden")
        << 2 << 2 << true << selectedItems << hiddenItems << expectedItems;


    selectedItems.clear(); hiddenItems.clear(); expectedItems.clear();
    selectedItems
        << (IntList() << 0)
        << (IntList() << 0 << 0)
        << (IntList() << 0 << 1)
        << (IntList() << 1)
        << (IntList() << 1 << 0)
        << (IntList() << 1 << 1);
    hiddenItems
        << (IntList() << 0);
    expectedItems
#if QT_VERSION >= 0x040100
        << (IntList() << 0)
        << (IntList() << 0 << 0)
        << (IntList() << 0 << 1)
#endif
        << (IntList() << 1)
        << (IntList() << 1 << 0)
        << (IntList() << 1 << 1);
    QTest::newRow("2 top with 2 children, closed, all selected, top0 hidden")
        << 2 << 2 << true << selectedItems << hiddenItems << expectedItems;

    selectedItems.clear(); hiddenItems.clear(); expectedItems.clear();
    selectedItems
        << (IntList() << 0)
        << (IntList() << 0 << 0)
        << (IntList() << 0 << 1)
        << (IntList() << 1)
        << (IntList() << 1 << 0)
        << (IntList() << 1 << 1);
    hiddenItems
        << (IntList() << 0 << 1)
        << (IntList() << 1);
    expectedItems
        << (IntList() << 0)
        << (IntList() << 0 << 0)
#if QT_VERSION >= 0x040100
        << (IntList() << 0 << 1)
        << (IntList() << 1)
        << (IntList() << 1 << 0)
        << (IntList() << 1 << 1)
#endif
        ;
    QTest::newRow("2 top with 2 children, closed, all selected, top0child1 and top1")
        << 2 << 2 << true << selectedItems << hiddenItems << expectedItems;

}

void tst_QTreeWidget::selectedItems()
{
    QFETCH(int, topLevel);
    QFETCH(int, children);
    QFETCH(bool, closeTopLevel);
    QFETCH(ListIntList, selectedItems);
    QFETCH(ListIntList, hiddenItems);
    QFETCH(ListIntList, expectedItems);

    // create items
    for (int t=0; t<topLevel; ++t) {
        QTreeWidgetItem *top = new QTreeWidgetItem(testWidget);
        top->setText(0, QString("top%1").arg(t));
        for (int c=0; c<children; ++c) {
            QTreeWidgetItem *child = new QTreeWidgetItem(top);
            child->setText(0, QString("top%1child%2").arg(t).arg(c));
        }
    }

    // set selected
    foreach (IntList itemPath, selectedItems) {
        QTreeWidgetItem *item = 0;
        foreach(int index, itemPath) {
            if (!item)
                item = testWidget->topLevelItem(index);
            else
                item = item->child(index);
        }
        testWidget->setItemSelected(item, true);
    }

    // hide rows
    foreach (IntList itemPath, hiddenItems) {
        QTreeWidgetItem *item = 0;
        foreach(int index, itemPath) {
            if (!item)
                item = testWidget->topLevelItem(index);
            else
                item = item->child(index);
        }
        testWidget->setItemHidden(item, true);
    }

    // open/close toplevel
    for (int i=0; i<testWidget->topLevelItemCount(); ++i) {
        if (closeTopLevel)
            testWidget->collapseItem(testWidget->topLevelItem(i));
        else
            testWidget->expandItem(testWidget->topLevelItem(i));
    }

    // check selectedItems
    QList<QTreeWidgetItem*> sel = testWidget->selectedItems();
    QCOMPARE(sel.count(), expectedItems.count());
    foreach (IntList itemPath, expectedItems) {
        QTreeWidgetItem *item = 0;
        foreach(int index, itemPath) {
            if (!item)
                item = testWidget->topLevelItem(index);
            else
                item = item->child(index);
        }
        QVERIFY(sel.contains(item));
    }

    // compare isSelected
    for (int t=0; t<testWidget->topLevelItemCount(); ++t) {
        QTreeWidgetItem *top = testWidget->topLevelItem(t);
        if (testWidget->isItemSelected(top))
            QVERIFY(sel.contains(top));
        for (int c=0; c<top->childCount(); ++c) {
            QTreeWidgetItem *child = top->child(c);
            if (testWidget->isItemSelected(child))
                QVERIFY(sel.contains(child));
        }
    }
}

void tst_QTreeWidget::itemAssignment()
{
    // create item with children and parent but not insert in the view
    QTreeWidgetItem grandParent;
    QTreeWidgetItem *parent = new QTreeWidgetItem(&grandParent);
    parent->setText(0, "foo");
    parent->setText(1, "bar");
    for (int i=0; i<5; ++i) {
        QTreeWidgetItem *child = new QTreeWidgetItem(parent);
        child->setText(0, "bingo");
        child->setText(1, "bango");
    }
    QCOMPARE(parent->parent(), &grandParent);
    QVERIFY(!parent->treeWidget());
    QCOMPARE(parent->columnCount(), 2);
    QCOMPARE(parent->text(0), QString("foo"));
    QCOMPARE(parent->childCount(), 5);
    QCOMPARE(parent->child(0)->parent(), parent);

    // create item which is inserted in the widget
    QTreeWidgetItem item(testWidget);
    item.setText(0, "baz");
    QVERIFY(!item.parent());
    QCOMPARE(item.treeWidget(), testWidget);
    QCOMPARE(item.columnCount(), 1);
    QCOMPARE(item.text(0), QString("baz"));
    QCOMPARE(item.childCount(), 0);

    // assign and test
    *parent = item;
    QCOMPARE(parent->parent(), &grandParent);
    QVERIFY(!parent->treeWidget());
    QCOMPARE(parent->columnCount(), 1);
    QCOMPARE(parent->text(0), QString("baz"));
    QCOMPARE(parent->childCount(), 5);
    QCOMPARE(parent->child(0)->parent(), parent);
}

void tst_QTreeWidget::clone_data()
{
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("topLevelIndex");
    QTest::addColumn<int>("childIndex");
    QTest::addColumn<QStringList>("topLevelText");
    QTest::addColumn<QStringList>("childText");
    QTest::addColumn<bool>("cloneChild");

    QTest::newRow("clone parent with child") << 0 << 0 << 0
                                          << (QStringList() << "some text")
                                          << (QStringList() << "more text")
                                          << false;

    QTest::newRow("clone child") << 0 << 0 << 0
                              << (QStringList() << "some text")
                              << (QStringList() << "more text")
                              << true;
}

void tst_QTreeWidget::clone()
{
    QFETCH(int, column);
    QFETCH(int, topLevelIndex);
    QFETCH(int, childIndex);
    QFETCH(QStringList, topLevelText);
    QFETCH(QStringList, childText);
    QFETCH(bool, cloneChild);

    for (int i = 0; i < topLevelText.count(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(testWidget);
        item->setText(column, topLevelText.at(i));
        for (int j = 0; j < childText.count(); ++j) {
            QTreeWidgetItem *child = new QTreeWidgetItem(item);
            child->setText(column, childText.at(j));
        }
    }

    QTreeWidgetItem *original = testWidget->topLevelItem(topLevelIndex);
    QTreeWidgetItem *copy = original->clone();
    QCOMPARE(copy->text(column), original->text(column));
    QCOMPARE(copy->childCount(), original->childCount());
    QVERIFY(!copy->parent());
    QVERIFY(!copy->treeWidget());

    QTreeWidgetItem *originalChild = original->child(childIndex);
    QTreeWidgetItem *copiedChild = cloneChild ? originalChild->clone() : copy->child(childIndex);
    QVERIFY(copiedChild != originalChild);
    QCOMPARE(copiedChild->text(column), originalChild->text(column));
    QCOMPARE(copiedChild->childCount(), originalChild->childCount());
    QCOMPARE(copiedChild->parent(), cloneChild ? 0 : copy);
    QVERIFY(!copiedChild->treeWidget());
    if (cloneChild)
        delete copiedChild;
    delete copy;
}

void tst_QTreeWidget::expand_data()
{
    QTest::addColumn<int>("topLevelIndex");
    QTest::addColumn<int>("topLevelCount");
    QTest::addColumn<int>("childIndex");
    QTest::addColumn<int>("childCount");

    QTest::newRow("the only test data for now") << 0 << 1 << 0 << 1;
}

void tst_QTreeWidget::expand()
{
    QFETCH(int, topLevelIndex);
    QFETCH(int, topLevelCount);
    QFETCH(int, childIndex);
    QFETCH(int, childCount);

    for (int i = 0; i < topLevelCount; ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(testWidget);
        for (int j = 0; j < childCount; ++j)
            new QTreeWidgetItem(item);
    }

    QTreeWidgetItem *topLevelItem = testWidget->topLevelItem(topLevelIndex);
    QTreeWidgetItem *childItem = topLevelItem->child(childIndex);

    QVERIFY(!testWidget->isItemExpanded(topLevelItem));
    testWidget->setItemExpanded(topLevelItem, true);
    QVERIFY(testWidget->isItemExpanded(topLevelItem));

    QVERIFY(!testWidget->isItemExpanded(childItem));
    testWidget->setItemExpanded(childItem, true);
    QVERIFY(testWidget->isItemExpanded(childItem));

    QVERIFY(testWidget->isItemExpanded(topLevelItem));
    testWidget->setItemExpanded(topLevelItem, false);
    QVERIFY(!testWidget->isItemExpanded(topLevelItem));

    QVERIFY(testWidget->isItemExpanded(childItem));
    testWidget->setItemExpanded(childItem, false);
    QVERIFY(!testWidget->isItemExpanded(childItem));
}

void tst_QTreeWidget::checkState_data()
{
}

void tst_QTreeWidget::checkState()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(testWidget);
    item->setCheckState(0, Qt::Unchecked);
    QTreeWidgetItem *firstChild = new QTreeWidgetItem(item);
    firstChild->setCheckState(0, Qt::Unchecked);
    QTreeWidgetItem *seccondChild = new QTreeWidgetItem(item);
    seccondChild->setCheckState(0, Qt::Unchecked);

    QCOMPARE(item->checkState(0), Qt::Unchecked);
    QCOMPARE(firstChild->checkState(0), Qt::Unchecked);
    QCOMPARE(seccondChild->checkState(0), Qt::Unchecked);

    firstChild->setCheckState(0, Qt::Checked);
    QCOMPARE(item->checkState(0), Qt::Unchecked);
    QCOMPARE(firstChild->checkState(0), Qt::Checked);
    QCOMPARE(seccondChild->checkState(0), Qt::Unchecked);

    item->setFlags(item->flags()|Qt::ItemIsTristate);
    QCOMPARE(item->checkState(0), Qt::PartiallyChecked);
    QCOMPARE(firstChild->checkState(0), Qt::Checked);
    QCOMPARE(seccondChild->checkState(0), Qt::Unchecked);

    seccondChild->setCheckState(0, Qt::Checked);
    QCOMPARE(item->checkState(0), Qt::Checked);
    QCOMPARE(firstChild->checkState(0), Qt::Checked);
    QCOMPARE(seccondChild->checkState(0), Qt::Checked);

    firstChild->setCheckState(0, Qt::Unchecked);
    seccondChild->setCheckState(0, Qt::Unchecked);
    QCOMPARE(item->checkState(0), Qt::Unchecked);
    QCOMPARE(firstChild->checkState(0), Qt::Unchecked);
    QCOMPARE(seccondChild->checkState(0), Qt::Unchecked);
}

void tst_QTreeWidget::findItems_data()
{
    QTest::addColumn<int>("column");
    QTest::addColumn<QStringList>("topLevelText");
    QTest::addColumn<QStringList>("childText");
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<QStringList>("resultText");

    QTest::newRow("find in toplevel")
        << 0
        << (QStringList() << "This is a text" << "This is another" << "This is the one")
        << (QStringList() << "A child" << "This is not the one" << "And yet another child")
        << "This is the one"
        << 1
        << (QStringList() << "This is the one");

    QTest::newRow("find child")
        << 0
        << (QStringList() << "This is a text" << "This is another" << "This is the one")
        << (QStringList() << "A child" << "This is not the one" << "And yet another child")
        << "A child"
        << 3 // once for each branch
        << (QStringList() << "A child");

}

void tst_QTreeWidget::findItems()
{
    QFETCH(int, column);
    QFETCH(QStringList, topLevelText);
    QFETCH(QStringList, childText);
    QFETCH(QString, pattern);
    QFETCH(int, resultCount);
    QFETCH(QStringList, resultText);

    for (int i = 0; i < topLevelText.count(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(testWidget);
        item->setText(column, topLevelText.at(i));
        for (int j = 0; j < childText.count(); ++j) {
            QTreeWidgetItem *child = new QTreeWidgetItem(item);
            child->setText(column, childText.at(j));
        }
    }

    QList<QTreeWidgetItem*> result = testWidget->findItems(pattern,
                                                           Qt::MatchExactly|Qt::MatchRecursive);
    QCOMPARE(result.count(), resultCount);

    for (int k = 0; k < result.count() && k < resultText.count(); ++k)
        QCOMPARE(result.at(k)->text(column), resultText.at(k));
}

void tst_QTreeWidget::findItemsInColumn()
{
    // Create 5 root items.
    for (int i = 0; i < 5; i++)
        new QTreeWidgetItem(testWidget, QStringList() << QString::number(i));

    // Create a child with two columns for each root item.
    for (int i = 0; i < 5; i++) {
        QTreeWidgetItem * const  parent = testWidget->topLevelItem(i);
        new QTreeWidgetItem(parent, QStringList() << QString::number(i * 10) << QString::number(i * 100));
    }

    // Recursively search column one for 400.
    QList<QTreeWidgetItem*> items = testWidget->findItems("400", Qt::MatchExactly|Qt::MatchRecursive, 1);
    QCOMPARE(items.count(), 1);
}

void tst_QTreeWidget::sortItems_data()
{
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("order");
    QTest::addColumn<QStringList>("topLevelText");
    QTest::addColumn<QStringList>("childText");
    QTest::addColumn<QStringList>("topLevelResult");
    QTest::addColumn<QStringList>("childResult");
    QTest::addColumn<IntList>("expectedTopRows");
    QTest::addColumn<IntList>("expectedChildRows");

    QTest::newRow("ascending order")
        << 0
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "c" << "d" << "a" << "b")
        << (QStringList() << "e" << "h" << "g" << "f")
        << (QStringList() << "a" << "b" << "c" << "d")
        << (QStringList() << "e" << "f" << "g" << "h")
        << (IntList() << 2 << 3 << 0 << 1)
        << (IntList() << 0 << 3 << 2 << 1);

    QTest::newRow("descending order")
        << 0
        << static_cast<int>(Qt::DescendingOrder)
        << (QStringList() << "c" << "d" << "a" << "b")
        << (QStringList() << "e" << "h" << "g" << "f")
        << (QStringList() << "d" << "c" << "b" << "a")
        << (QStringList() << "h" << "g" << "f" << "e")
        << (IntList() << 1 << 0 << 3 << 2)
        << (IntList() << 3 << 0 << 1 << 2);
}

void tst_QTreeWidget::sortItems()
{
    QFETCH(int, column);
    QFETCH(int, order);
    QFETCH(QStringList, topLevelText);
    QFETCH(QStringList, childText);
    QFETCH(QStringList, topLevelResult);
    QFETCH(QStringList, childResult);
    QFETCH(IntList, expectedTopRows);
    QFETCH(IntList, expectedChildRows);
    testWidget->setSortingEnabled(false);

    for (int i = 0; i < topLevelText.count(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(testWidget);
        item->setText(column, topLevelText.at(i));
        for (int j = 0; j < childText.count(); ++j) {
            QTreeWidgetItem *child = new QTreeWidgetItem(item);
            child->setText(column, childText.at(j));
        }
    }

    QAbstractItemModel *model = testWidget->model();
    QList<QPersistentModelIndex> tops;
    for (int r = 0; r < model->rowCount(QModelIndex()); ++r) {
        QPersistentModelIndex p = model->index(r, 0, QModelIndex());
        tops << p;
    }
    QList<QPersistentModelIndex> children;
    for (int s = 0; s < model->rowCount(tops.first()); ++s) {
        QPersistentModelIndex c = model->index(s, 0, tops.first());
        children << c;
    }

    testWidget->sortItems(column, static_cast<Qt::SortOrder>(order));
    QCOMPARE(testWidget->sortColumn(), column);

    for (int k = 0; k < topLevelResult.count(); ++k) {
        QTreeWidgetItem *item = testWidget->topLevelItem(k);
        QCOMPARE(item->text(column), topLevelResult.at(k));
        for (int l = 0; l < childResult.count(); ++l)
            QCOMPARE(item->child(l)->text(column), childResult.at(l));
    }

    for (int m = 0; m < tops.count(); ++m)
        QCOMPARE(tops.at(m).row(), expectedTopRows.at(m));
    for (int n = 0; n < children.count(); ++n)
        QCOMPARE(children.at(n).row(), expectedChildRows.at(n));
}

void tst_QTreeWidget::deleteItems_data()
{
    QTest::addColumn<int>("topLevelCount");
    QTest::addColumn<int>("childCount");
    QTest::addColumn<int>("grandChildCount");

    QTest::addColumn<int>("deleteTopLevelCount");
    QTest::addColumn<int>("deleteChildCount");
    QTest::addColumn<int>("deleteGrandChildCount");

    QTest::addColumn<int>("expectedTopLevelCount");
    QTest::addColumn<int>("expectedChildCount");
    QTest::addColumn<int>("expectedGrandChildCount");

    QTest::addColumn<int>("persistentRow");
    QTest::addColumn<int>("persistentColumn");
    QTest::addColumn<bool>("persistentIsValid");

    QTest::newRow("start with 10, delete 1")
        << 10 << 10 << 10
        << 1 << 1 << 1
        << 9 << 9 << 9
        << 0 << 0 << false;
    QTest::newRow("start with 10, delete 5")
        << 10 << 10 << 10
        << 5 << 5 << 5
        << 5 << 5 << 5
        << 0 << 0 << false;
    QTest::newRow("mixed")
        << 10 << 13 << 7
        << 3 << 7 << 4
        << 7 << 6 << 3
        << 0 << 0 << false;
    QTest::newRow("all")
        << 10 << 10 << 10
        << 10 << 10 << 10
        << 0 << 0 << 0
        << 0 << 0 << false;
}

void tst_QTreeWidget::deleteItems()
{
    QFETCH(int, topLevelCount);
    QFETCH(int, childCount);
    QFETCH(int, grandChildCount);

    QFETCH(int, deleteTopLevelCount);
    QFETCH(int, deleteChildCount);
    QFETCH(int, deleteGrandChildCount);

    QFETCH(int, expectedTopLevelCount);
    QFETCH(int, expectedChildCount);
    QFETCH(int, expectedGrandChildCount);

    QFETCH(int, persistentRow);
    QFETCH(int, persistentColumn);
    QFETCH(bool, persistentIsValid);

    for (int i = 0; i < topLevelCount; ++i) {
        QTreeWidgetItem *top = new QTreeWidgetItem(testWidget);
        for (int j = 0; j < childCount; ++j) {
            QTreeWidgetItem *child = new QTreeWidgetItem(top);
            for (int k = 0; k < grandChildCount; ++k) {
                new QTreeWidgetItem(child);
            }
        }
    }

    QPersistentModelIndex persistent = testWidget->model()->index(persistentRow,
                                                                  persistentColumn);
    QVERIFY(persistent.isValid());

    QTreeWidgetItem *top = testWidget->topLevelItem(0);
    QTreeWidgetItem *child = top->child(0);

    for (int n = 0; n < deleteGrandChildCount; ++n)
        delete child->child(0);
    QCOMPARE(child->childCount(), expectedGrandChildCount);

    for (int m = 0; m < deleteChildCount; ++m)
        delete top->child(0);
    QCOMPARE(top->childCount(), expectedChildCount);

    for (int l = 0; l < deleteTopLevelCount; ++l)
        delete testWidget->topLevelItem(0);
    QCOMPARE(testWidget->topLevelItemCount(), expectedTopLevelCount);

    QCOMPARE(persistent.isValid(), persistentIsValid);
}

void tst_QTreeWidget::itemStreaming_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("toolTip");
    QTest::addColumn<int>("column");

    QTest::newRow("Data") << "item text" << "tool tip text" << 0;
}

void tst_QTreeWidget::itemStreaming()
{
    QFETCH(QString, text);
    QFETCH(QString, toolTip);
    QFETCH(int, column);

    QTreeWidgetItem item(testWidget);
    QCOMPARE(item.text(column), QString());
    QCOMPARE(item.toolTip(column), QString());

    item.setText(column, text);
    item.setToolTip(column, toolTip);
    QCOMPARE(item.text(column), text);
    QCOMPARE(item.toolTip(column), toolTip);

    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << item;

    QTreeWidgetItem item2(testWidget);
    QCOMPARE(item2.text(column), QString());
    QCOMPARE(item2.toolTip(column), QString());

    QVERIFY(!buffer.isEmpty());

    QDataStream in(&buffer, QIODevice::ReadOnly);
    in >> item2;
    QCOMPARE(item2.text(column), text);
    QCOMPARE(item2.toolTip(column), toolTip);
}

void tst_QTreeWidget::insertTopLevelItems_data()
{
    QTest::addColumn<QStringList>("initialText");
    QTest::addColumn<QStringList>("insertText");
    QTest::addColumn<int>("insertTopLevelIndex");
    QTest::addColumn<int>("expectedTopLevelIndex");
    QTest::addColumn<int>("insertChildIndex");
    QTest::addColumn<int>("expectedChildIndex");

    QStringList initial = (QStringList() << "foo" << "bar");
    QStringList insert = (QStringList() << "baz");

    QTest::newRow("Insert at count") << initial << insert
                                     << initial.count() << initial.count()
                                     << initial.count() << initial.count();
    QTest::newRow("Insert in the middle") << initial << insert
                                          << (initial.count() / 2) << (initial.count() / 2)
                                          << (initial.count() / 2) << (initial.count() / 2);
    QTest::newRow("Insert less than 0") << initial << insert
                                        << -1 << -1
                                        << -1 << -1;
    QTest::newRow("Insert beyond count") << initial << insert
                                         << initial.count() + 1 << -1
                                         << initial.count() + 1 << -1;
}

void tst_QTreeWidget::insertTopLevelItems()
{
    QFETCH(QStringList, initialText);
    QFETCH(QStringList, insertText);
    QFETCH(int, insertTopLevelIndex);
    QFETCH(int, expectedTopLevelIndex);
    QFETCH(int, insertChildIndex);
    QFETCH(int, expectedChildIndex);
    testWidget->setSortingEnabled(false);

    { // insert the initial items
        QCOMPARE(testWidget->topLevelItemCount(), 0);
        for (int i = 0; i < initialText.count(); ++i) {
            QTreeWidgetItem *top = new QTreeWidgetItem(QStringList(initialText.at(i)));
            testWidget->addTopLevelItem(top);
            QCOMPARE(testWidget->indexOfTopLevelItem(top), i);
        }
        QCOMPARE(testWidget->topLevelItemCount(), initialText.count());
    }

    { // test adding children
        QTreeWidgetItem *topLevel = testWidget->topLevelItem(0);
        for (int i = 0; i < initialText.count(); ++i)
            topLevel->addChild(new QTreeWidgetItem(QStringList(initialText.at(i))));
        QCOMPARE(topLevel->childCount(), initialText.count());
    }

    { // test adding more top level items
        testWidget->insertTopLevelItem(insertTopLevelIndex, new QTreeWidgetItem(QStringList(insertText.at(0))));
        if (expectedTopLevelIndex == -1) {
            QCOMPARE(testWidget->topLevelItemCount(), initialText.count());
        } else {
            QTreeWidgetItem *item = testWidget->topLevelItem(expectedTopLevelIndex);
            QVERIFY(item != 0);
            QCOMPARE(item->text(0), insertText.at(0));
            QCOMPARE(testWidget->indexOfTopLevelItem(item), expectedTopLevelIndex);
        }
    }

    { // test adding more children
        QTreeWidgetItem *topLevel = testWidget->topLevelItem(0);
        QVERIFY(topLevel != 0);
        topLevel->insertChild(insertChildIndex, new QTreeWidgetItem(QStringList(insertText.at(0))));
        if (expectedChildIndex == -1) {
            QCOMPARE(topLevel->childCount(), initialText.count());
        } else {
            QTreeWidgetItem *item = topLevel->child(expectedChildIndex);
            QVERIFY(item != 0);
            QCOMPARE(item->text(0), insertText.at(0));
        }
    }
}

static void fillTreeWidget(QTreeWidgetItem *parent, int rows)
{
    const int columns = parent->treeWidget()->columnCount();
    for (int r = 0; r < rows; ++r) {
        QTreeWidgetItem *w = new QTreeWidgetItem(parent);
        for ( int c = 0; c < columns; ++c ) {
            QString s = QString("[r:%1,c:%2]").arg(r).arg(c);
            w->setText(c, s);
        }
        fillTreeWidget(w, rows - r - 1);
    }
}

static void fillTreeWidget(QTreeWidget *tree, int rows)
{
    for (int r = 0; r < rows; ++r) {
        QTreeWidgetItem *w = new QTreeWidgetItem();
        for ( int c = 0; c < tree->columnCount(); ++c ) {
            QString s = QString("[r:%1,c:%2]").arg(r).arg(c);
            w->setText(c, s);
        }
        tree->insertTopLevelItem(r, w);
        fillTreeWidget(w, rows - r - 1);
    }
}

void tst_QTreeWidget::keyboardNavigation()
{
    int rows = 8;

    fillTreeWidget(testWidget, rows);

    QVector<Qt::Key> keymoves;
    keymoves << Qt::Key_Down << Qt::Key_Right << Qt::Key_Right << Qt::Key_Left
	     << Qt::Key_Down << Qt::Key_Down << Qt::Key_Down << Qt::Key_Down
	     << Qt::Key_Right << Qt::Key_Right << Qt::Key_Right
	     << Qt::Key_Left << Qt::Key_Up << Qt::Key_Left << Qt::Key_Left
	     << Qt::Key_Up << Qt::Key_Down << Qt::Key_Up << Qt::Key_Up
	     << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up
             << Qt::Key_Down << Qt::Key_Right << Qt::Key_Down << Qt::Key_Down
             << Qt::Key_Down << Qt::Key_Right << Qt::Key_Down << Qt::Key_Down
	     << Qt::Key_Left << Qt::Key_Left << Qt::Key_Up << Qt::Key_Down
             << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up << Qt::Key_Left
             << Qt::Key_Down;

    int row    = 0;
    QTreeWidgetItem *item = testWidget->topLevelItem(0);
    testWidget->setCurrentItem(item);
    QCOMPARE(testWidget->currentItem(), item);
    QApplication::instance()->processEvents();

    for (int i = 0; i < keymoves.size(); ++i) {
        Qt::Key key = keymoves.at(i);
        QTest::keyClick(testWidget, key);
        QApplication::instance()->processEvents();

        switch (key) {
        case Qt::Key_Up:
	    if (row > 0) {
                if (item->parent())
                    item = item->parent()->child(row - 1);
                else
                    item = testWidget->topLevelItem(row - 1);
		row -= 1;
	    } else if (item->parent()) {
		item = item->parent();
		row = item->parent() ? item->parent()->indexOfChild(item) : 0;
	    }
            break;
        case Qt::Key_Down:
            if (testWidget->isItemExpanded(item)) {
                row = 0;
                item = item->child(row);
            } else {
                row = qMin(rows - 1, row + 1);
                if (item->parent())
                    item = item->parent()->child(row);
                else
                    item = testWidget->topLevelItem(row);
            }
            break;
        case Qt::Key_Left:
	    QVERIFY(!testWidget->isItemExpanded(item));
            break;
        case Qt::Key_Right:
	    QVERIFY(testWidget->isItemExpanded(item));
	    break;
        default:
            QVERIFY(false);
        }

        QTreeWidgetItem *current = testWidget->currentItem();
        QCOMPARE(current->text(0), QString("[r:%1,c:0]").arg(row));
        if (current->parent())
            QCOMPARE(current->parent()->indexOfChild(current), row);
        else
            QCOMPARE(testWidget->indexOfTopLevelItem(current), row);
    }
}

void tst_QTreeWidget::scrollToItem()
{
#if QT_VERSION < 0x040100
    QSKIP("This behaviour will be merged from main in 4.1.0.", SkipAll);
#else
    // Check if all parent nodes of the item found are expanded.
    // Reported in task #78761
    QTreeWidgetItem *bar;
    QTreeWidgetItem *search;
    for (int i=0; i<2; ++i) {
        bar = new QTreeWidgetItem(testWidget);
        bar->setText(0, QString::number(i));

        for (int j=0; j<2; ++j) {
            QTreeWidgetItem *foo = new QTreeWidgetItem(bar);
            foo->setText(0, bar->text(0) + QString::number(j));

            for (int k=0; k<2; ++k) {
                QTreeWidgetItem *yo = new QTreeWidgetItem(foo);
                yo->setText(0, foo->text(0) + QString::number(k));
                search = yo;
            }
        }
    }

    testWidget->setHeaderLabels(QStringList() << "foo");
    testWidget->scrollToItem(search);
    QVERIFY(search->text(0) == "111");

    bar = search->parent();
    QVERIFY(testWidget->isItemExpanded(bar));
    bar = bar->parent();
    QVERIFY(testWidget->isItemExpanded(bar));
#endif
}

// From task #85413
void tst_QTreeWidget::setSortingEnabled()
{
    QStringList hl;
    hl << "ID";
    testWidget->setColumnCount(hl.count());
    testWidget->setHeaderLabels(hl);
    
    testWidget->setSortingEnabled(true);
    QCOMPARE(testWidget->isSortingEnabled(), true);
    QCOMPARE(testWidget->isSortingEnabled(), testWidget->header()->isSortIndicatorShown());
    
    // Make sure we do it twice
    testWidget->setSortingEnabled(true);
    QCOMPARE(testWidget->isSortingEnabled(), true);
    QCOMPARE(testWidget->isSortingEnabled(), testWidget->header()->isSortIndicatorShown());

    testWidget->setSortingEnabled(false);
    QCOMPARE(testWidget->isSortingEnabled(), false);
    QCOMPARE(testWidget->isSortingEnabled(), testWidget->header()->isSortIndicatorShown());

    testWidget->setSortingEnabled(false);
    QCOMPARE(testWidget->isSortingEnabled(), false);
    QCOMPARE(testWidget->isSortingEnabled(), testWidget->header()->isSortIndicatorShown());

    // And back again so that we make sure that we test the transition from false to true
    testWidget->setSortingEnabled(true);
    QCOMPARE(testWidget->isSortingEnabled(), true);
    QCOMPARE(testWidget->isSortingEnabled(), testWidget->header()->isSortIndicatorShown());

    testWidget->setSortingEnabled(true);
    QCOMPARE(testWidget->isSortingEnabled(), true);
    QCOMPARE(testWidget->isSortingEnabled(), testWidget->header()->isSortIndicatorShown());

    testWidget->setSortingEnabled(false);
}

void tst_QTreeWidget::addChild()
{
    QTreeWidget tree;
    for (int x = 0; x < 2; ++x) {
        QTreeWidget *view = x ? &tree : static_cast<QTreeWidget*>(0);
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)view);
        QCOMPARE(item->childCount(), 0);
        
        // try to add 0
        item->addChild(0);
        QCOMPARE(item->childCount(), 0);
        QCOMPARE(item->indexOfChild(0), -1);
        
        // add one at a time
        QList<QTreeWidgetItem*> children;
        for (int i = 0; i < 10; ++i) {
            QTreeWidgetItem *child = new QTreeWidgetItem();
            item->addChild(child);
            QCOMPARE(item->childCount(), i+1);
            QCOMPARE(item->child(i), child);
            QCOMPARE(item->indexOfChild(child), i);
            QCOMPARE(child->parent(), item);
            QCOMPARE(child->treeWidget(), view);
            item->addChild(child);
            QCOMPARE(item->childCount(), i+1);
            children.append(child);
        }
        
        // take them all
        QList<QTreeWidgetItem*> taken = item->takeChildren();
        QCOMPARE(taken, children);
        QCOMPARE(item->childCount(), 0);
        for (int i = 0; i < taken.count(); ++i) {
            QCOMPARE(taken.at(i)->parent(), static_cast<QTreeWidgetItem*>(0));
            QCOMPARE(taken.at(i)->treeWidget(), static_cast<QTreeWidget*>(0));
            item->addChild(taken.at(i)); // re-add
        }

        // delete one at a time
        while (!children.isEmpty()) {
            QTreeWidgetItem *ti = children.takeFirst();
            delete ti;
            QCOMPARE(item->childCount(), children.count());
            for (int i = 0; i < children.count(); ++i)
                QCOMPARE(item->child(i), children.at(i));
        }
        
        // add none
        {
            int count = item->childCount();
            item->addChildren(QList<QTreeWidgetItem*>());
            QCOMPARE(item->childCount(), count);
        }
        
        // add many at a time
        const int count = 10;
        for (int i = 0; i < 100; i += count) {
            QList<QTreeWidgetItem*> list;
            for (int j = 0; j < count; ++j)
                list << new QTreeWidgetItem(QStringList() << QString("%0").arg(j));
            item->addChildren(list);
            QCOMPARE(item->childCount(), count + i);
            for (int j = 0; j < count; ++j) {
                QCOMPARE(item->child(i+j), list.at(j));
                QCOMPARE(item->child(i+j)->parent(), item);
            }
            
            item->addChildren(list);
            QCOMPARE(item->childCount(), count + i);
        }

        if (!view)
            delete item;
    }
}

void tst_QTreeWidget::setData()
{
    {
        QTreeWidgetItem *headerItem = new QTreeWidgetItem();
        headerItem->setText(0, "Item1");
        testWidget->setHeaderItem(headerItem);
        
        QSignalSpy headerDataChangedSpy(
            testWidget->model(), SIGNAL(headerDataChanged(Qt::Orientation, int, int)));
        QSignalSpy dataChangedSpy(
            testWidget->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)));
        QSignalSpy itemChangedSpy(
            testWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)));
        headerItem->setText(0, "test");
        QCOMPARE(dataChangedSpy.count(), 0);
        QCOMPARE(headerDataChangedSpy.count(), 1);
        QCOMPARE(itemChangedSpy.count(), 0); // no itemChanged() signal for header item
        
        headerItem->setData(-1, -1, QVariant());
    }

    {
        QSignalSpy itemChangedSpy(
            testWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)));
        QTreeWidgetItem *item = new QTreeWidgetItem();
        testWidget->addTopLevelItem(item);
        for (int x = 0; x < 2; ++x) {
            for (int i = 1; i <= 2; ++i) {
                for (int j = 0; j < 5; ++j) {
                    QVariantList args;
                    QString text = QString("text %0").arg(i);
                    item->setText(j, text);
                    QCOMPARE(item->text(j), text);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setText(j, text);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    QPixmap pixmap(32, 32);
                    pixmap.fill((i == 1) ? Qt::red : Qt::green);
                    QIcon icon(pixmap);
                    item->setIcon(j, icon);
                    QCOMPARE(item->icon(j), icon);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setIcon(j, icon);
                    // #### shouldn't cause dataChanged()
                    QCOMPARE(itemChangedSpy.count(), 1);
                    itemChangedSpy.clear();
                    
                    QString toolTip = QString("toolTip %0").arg(i);
                    item->setToolTip(j, toolTip);
                    QCOMPARE(item->toolTip(j), toolTip);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setToolTip(j, toolTip);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    QString statusTip = QString("statusTip %0").arg(i);
                    item->setStatusTip(j, statusTip);
                    QCOMPARE(item->statusTip(j), statusTip);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setStatusTip(j, statusTip);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    QString whatsThis = QString("whatsThis %0").arg(i);
                    item->setWhatsThis(j, whatsThis);
                    QCOMPARE(item->whatsThis(j), whatsThis);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setWhatsThis(j, whatsThis);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    QSize sizeHint(64*i, 48*i);
                    item->setSizeHint(j, sizeHint);
                    QCOMPARE(item->sizeHint(j), sizeHint);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setSizeHint(j, sizeHint);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    QFont font;
                    item->setFont(j, font);
                    QCOMPARE(item->font(j), font);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setFont(j, font);
                    QCOMPARE(itemChangedSpy.count(), 0);
        
                    Qt::Alignment textAlignment((i == 1)
                                                ? Qt::AlignLeft|Qt::AlignVCenter
                                                : Qt::AlignRight);
                    item->setTextAlignment(j, textAlignment);
                    QCOMPARE(item->textAlignment(j), int(textAlignment));
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setTextAlignment(j, textAlignment);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    QColor backgroundColor((i == 1) ? Qt::blue : Qt::yellow);
                    item->setBackground(j, backgroundColor);
                    QCOMPARE(item->background(j).color(), backgroundColor);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setBackground(j, backgroundColor);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    QColor textColor((i == i) ? Qt::green : Qt::cyan);
                    item->setTextColor(j, textColor);
                    QCOMPARE(item->textColor(j), textColor);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setTextColor(j, textColor);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    Qt::CheckState checkState((i == 1) ? Qt::PartiallyChecked : Qt::Checked);
                    item->setCheckState(j, checkState);
                    QCOMPARE(item->checkState(j), checkState);
                    QCOMPARE(itemChangedSpy.count(), 1);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setCheckState(j, checkState);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    QCOMPARE(item->text(j), text);
                    QCOMPARE(item->icon(j), icon);
                    QCOMPARE(item->toolTip(j), toolTip);
                    QCOMPARE(item->statusTip(j), statusTip);
                    QCOMPARE(item->whatsThis(j), whatsThis);
                    QCOMPARE(item->sizeHint(j), sizeHint);
                    QCOMPARE(item->font(j), font);
                    QCOMPARE(item->textAlignment(j), int(textAlignment));
                    QCOMPARE(item->background(j).color(), backgroundColor);
                    QCOMPARE(item->textColor(j), textColor);
                    QCOMPARE(item->checkState(j), checkState);
                    
                    QCOMPARE(qvariant_cast<QString>(item->data(j, Qt::DisplayRole)), text);
                    QCOMPARE(qvariant_cast<QIcon>(item->data(j, Qt::DecorationRole)), icon);
                    QCOMPARE(qvariant_cast<QString>(item->data(j, Qt::ToolTipRole)), toolTip);
                    QCOMPARE(qvariant_cast<QString>(item->data(j, Qt::StatusTipRole)), statusTip);
                    QCOMPARE(qvariant_cast<QString>(item->data(j, Qt::WhatsThisRole)), whatsThis);
                    QCOMPARE(qvariant_cast<QSize>(item->data(j, Qt::SizeHintRole)), sizeHint);
                    QCOMPARE(qvariant_cast<QFont>(item->data(j, Qt::FontRole)), font);
                    QCOMPARE(qvariant_cast<int>(item->data(j, Qt::TextAlignmentRole)), int(textAlignment));
                    QCOMPARE(qvariant_cast<QBrush>(item->data(j, Qt::BackgroundColorRole)), QBrush(backgroundColor));
                    QCOMPARE(qvariant_cast<QBrush>(item->data(j, Qt::BackgroundRole)), QBrush(backgroundColor));
                    QCOMPARE(qvariant_cast<QColor>(item->data(j, Qt::TextColorRole)), textColor);
                    QCOMPARE(qvariant_cast<int>(item->data(j, Qt::CheckStateRole)), int(checkState));
                    
                    item->setBackground(j, pixmap);
                    QCOMPARE(item->background(j).texture(), pixmap);
                    QCOMPARE(qvariant_cast<QBrush>(item->data(j, Qt::BackgroundRole)).texture(), pixmap);
                    args = itemChangedSpy.takeFirst();
                    QCOMPARE(qvariant_cast<QTreeWidgetItem*>(args.at(0)), item);
                    QCOMPARE(qvariant_cast<int>(args.at(1)), j);
                    item->setBackground(j, pixmap);
                    QCOMPARE(itemChangedSpy.count(), 0);
                    
                    item->setData(j, Qt::DisplayRole, QVariant());
                    item->setData(j, Qt::DecorationRole, QVariant());
                    item->setData(j, Qt::ToolTipRole, QVariant());
                    item->setData(j, Qt::StatusTipRole, QVariant());
                    item->setData(j, Qt::WhatsThisRole, QVariant());
                    item->setData(j, Qt::SizeHintRole, QVariant());
                    item->setData(j, Qt::FontRole, QVariant());
                    item->setData(j, Qt::TextAlignmentRole, QVariant());
                    item->setData(j, Qt::BackgroundColorRole, QVariant());
                    item->setData(j, Qt::TextColorRole, QVariant());
                    item->setData(j, Qt::CheckStateRole, QVariant());
                    QCOMPARE(itemChangedSpy.count(), 11);
                    itemChangedSpy.clear();
                    
                    QCOMPARE(item->data(j, Qt::DisplayRole).toString(), QString());
                    QCOMPARE(item->data(j, Qt::DecorationRole), QVariant());
                    QCOMPARE(item->data(j, Qt::ToolTipRole), QVariant());
                    QCOMPARE(item->data(j, Qt::StatusTipRole), QVariant());
                    QCOMPARE(item->data(j, Qt::WhatsThisRole), QVariant());
                    QCOMPARE(item->data(j, Qt::SizeHintRole), QVariant());
                    QCOMPARE(item->data(j, Qt::FontRole), QVariant());
                    QCOMPARE(item->data(j, Qt::TextAlignmentRole), QVariant());
                    QCOMPARE(item->data(j, Qt::BackgroundColorRole), QVariant());
                    QCOMPARE(item->data(j, Qt::BackgroundRole), QVariant());
                    QCOMPARE(item->data(j, Qt::TextColorRole), QVariant());
                    QCOMPARE(item->data(j, Qt::CheckStateRole), QVariant());
                }
            }
        }
        delete item;
    }
}

void tst_QTreeWidget::match()
{
    QTreeWidget tree;
    QModelIndexList list = tree.model()->match(QModelIndex(), Qt::DisplayRole, QString());

    QVERIFY(list.isEmpty());
}

void tst_QTreeWidget::columnCount()
{
    int columnCountBefore = testWidget->columnCount();
    testWidget->setColumnCount(-1);
    QCOMPARE(testWidget->columnCount(), columnCountBefore);
}

void tst_QTreeWidget::setHeaderLabels()
{
    QStringList list = QString("a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z").split(",");
    testWidget->setHeaderLabels(list);
    QCOMPARE(testWidget->header()->count(), list.count());
}

void tst_QTreeWidget::setHeaderItem()
{
    testWidget->setHeaderItem(0);
    QTreeWidgetItem *headerItem = new QTreeWidgetItem();

    testWidget->setColumnCount(0);
    QCOMPARE(testWidget->header()->count(), 0);
    QCOMPARE(testWidget->columnCount(), 0);

    headerItem->setText(0, "0");
    headerItem->setText(1, "1");
    testWidget->setHeaderItem(headerItem);
    QCOMPARE(testWidget->headerItem(), headerItem);
    QCOMPARE(headerItem->treeWidget(), testWidget);

    QCOMPARE(testWidget->header()->count(), 2);
    QCOMPARE(testWidget->columnCount(), 2);

    headerItem->setText(2, "2");
    QCOMPARE(testWidget->header()->count(), 3);
    QCOMPARE(testWidget->columnCount(), 3);

    delete headerItem;
    testWidget->setColumnCount(3);
    testWidget->setColumnCount(5);
    QCOMPARE(testWidget->model()->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), QString("1"));
    QCOMPARE(testWidget->model()->headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("2"));
    QCOMPARE(testWidget->model()->headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QString("3"));
    QCOMPARE(testWidget->model()->headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), QString("4"));
    QCOMPARE(testWidget->model()->headerData(4, Qt::Horizontal, Qt::DisplayRole).toString(), QString("5"));

    headerItem = new QTreeWidgetItem();
    testWidget->setHeaderItem(headerItem);
    testWidget->model()->insertColumns(0, 5, QModelIndex());
    QCOMPARE(testWidget->model()->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), QString("1"));
    QCOMPARE(testWidget->model()->headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QString("2"));
    QCOMPARE(testWidget->model()->headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QString("3"));
    QCOMPARE(testWidget->model()->headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), QString("4"));
    QCOMPARE(testWidget->model()->headerData(4, Qt::Horizontal, Qt::DisplayRole).toString(), QString("5"));
}

void tst_QTreeWidget::itemWidget_data()
{
    editItem_data();
}

void tst_QTreeWidget::itemWidget()
{
    QFETCH(TreeItemList, topLevelItems);

    QTreeWidget tree;
    populate(&tree, topLevelItems, new TreeItem(QStringList() << "1" << "2"));
    tree.show();

    for (int x = 0; x < 2; ++x) {
        QTreeWidgetItemIterator it(&tree);
        while (QTreeWidgetItem *item = (*it++)) {
            for (int col = 0; col < item->columnCount(); ++col) {
                if (x == 0) {
                    QCOMPARE(tree.itemWidget(item, col), static_cast<QWidget*>(0));
                    QWidget *editor = new QLineEdit();
                    tree.setItemWidget(item, col, editor);
                    QCOMPARE(tree.itemWidget(item, col), editor);
                    tree.setItemWidget(item, col, 0);
                    QCOMPARE(tree.itemWidget(item, col), static_cast<QWidget*>(0));
                } else {
                    // ### should you really be able to open a persistent
                    //     editor for an item that isn't editable??
                    tree.openPersistentEditor(item, col);
                    QWidget *editor = tree.findChild<QLineEdit*>();
                    QVERIFY(editor != 0);
                    tree.closePersistentEditor(item, col);
                }
            }
        }
    }
}

#if QT_VERSION >= 0x040200
void tst_QTreeWidget::insertItemsWithSorting_data()
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

void tst_QTreeWidget::insertItemsWithSorting()
{
    QFETCH(int, sortOrder);
    QFETCH(QStringList, initialItems);
    QFETCH(QStringList, insertItems);
    QFETCH(QStringList, expectedItems);
    QFETCH(IntList, expectedRows);

    for (int method = 0; method < 5; ++method) {
        QTreeWidget w;
        w.setSortingEnabled(true);
        w.sortItems(0, static_cast<Qt::SortOrder>(sortOrder));
        for (int i = 0; i < initialItems.count(); ++i)
            w.addTopLevelItem(new QTreeWidgetItem(QStringList() << initialItems.at(i)));

        QAbstractItemModel *model = w.model();
        QList<QPersistentModelIndex> persistent;
        for (int j = 0; j < model->rowCount(QModelIndex()); ++j)
            persistent << model->index(j, 0, QModelIndex());

        switch (method) {
            case 0:
                // insert using item constructor
                for (int i = 0; i < insertItems.size(); ++i)
                    new QTreeWidgetItem(&w, QStringList() << insertItems.at(i));
                break;
            case 1:
            {
                // insert using insertTopLevelItems()
                QList<QTreeWidgetItem*> lst;
                for (int i = 0; i < insertItems.size(); ++i)
                    lst << new QTreeWidgetItem(QStringList() << insertItems.at(i));
                w.insertTopLevelItems(0, lst);
                break;
            }
            case 2:
                // insert using insertTopLevelItem()
                for (int i = 0; i < insertItems.size(); ++i)
                    w.insertTopLevelItem(0, new QTreeWidgetItem(QStringList() << insertItems.at(i)));
                break;
            case 3:
            {
                // insert using addTopLevelItems()
                QList<QTreeWidgetItem*> lst;
                for (int i = 0; i < insertItems.size(); ++i)
                    lst << new QTreeWidgetItem(QStringList() << insertItems.at(i));
                w.addTopLevelItems(lst);
                break;
            }
            case 4:
                // insert using addTopLevelItem()
                for (int i = 0; i < insertItems.size(); ++i)
                    w.addTopLevelItem(new QTreeWidgetItem(QStringList() << insertItems.at(i)));
                break;
        }
        QCOMPARE(w.topLevelItemCount(), expectedItems.count());
        for (int i = 0; i < w.topLevelItemCount(); ++i)
            QCOMPARE(w.topLevelItem(i)->text(0), expectedItems.at(i));

        for (int k = 0; k < persistent.count(); ++k)
            QCOMPARE(persistent.at(k).row(), expectedRows.at(k));
    }
}

void tst_QTreeWidget::changeDataWithSorting_data()
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

void tst_QTreeWidget::changeDataWithSorting()
{
    QFETCH(int, sortOrder);
    QFETCH(QStringList, initialItems);
    QFETCH(int, itemIndex);
    QFETCH(QString, newValue);
    QFETCH(QStringList, expectedItems);
    QFETCH(IntList, expectedRows);
    QFETCH(bool, reorderingExpected);

    QTreeWidget w;
    w.setSortingEnabled(true);
    w.sortItems(0, static_cast<Qt::SortOrder>(sortOrder));
    for (int i = 0; i < initialItems.count(); ++i)
        w.addTopLevelItem(new QTreeWidgetItem(QStringList() << initialItems.at(i)));

    QAbstractItemModel *model = w.model();
    QList<QPersistentModelIndex> persistent;
    for (int j = 0; j < model->rowCount(QModelIndex()); ++j)
        persistent << model->index(j, 0, QModelIndex());

    QSignalSpy dataChangedSpy(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)));
    QSignalSpy layoutChangedSpy(model, SIGNAL(layoutChanged()));

    QTreeWidgetItem *item = w.topLevelItem(itemIndex);
    item->setText(0, newValue);
    for (int i = 0; i < expectedItems.count(); ++i)
        QCOMPARE(w.topLevelItem(i)->text(0), expectedItems.at(i));

    for (int k = 0; k < persistent.count(); ++k)
        QCOMPARE(persistent.at(k).row(), expectedRows.at(k));

    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(layoutChangedSpy.count(), reorderingExpected ? 1 : 0);
}
#endif // QT_VERSION

void tst_QTreeWidget::itemOperatorLessThan()
{
    QTreeWidget tw;
    tw.setColumnCount(2);
#if QT_VERSION >= 0x040200 // crashes below 4.2
    {
        QTreeWidgetItem item1(0);
        QTreeWidgetItem item2(0);
        QCOMPARE(item1 < item2, false);
        item1.setText(1, "a");
        item2.setText(1, "b");
        QCOMPARE(item1 < item2, false);
        item1.setText(0, "a");
        item2.setText(0, "b");
        QCOMPARE(item1 < item2, true);
    }
#endif
    {
        QTreeWidgetItem item1(&tw);
        QTreeWidgetItem item2(&tw);
        QCOMPARE(item1 < item2, false);
        item1.setText(1, "a");
        item2.setText(1, "b");
        QCOMPARE(item1 < item2, false);
        item1.setText(0, "a");
        item2.setText(0, "b");
        QCOMPARE(item1 < item2, true);
        tw.sortItems(1, Qt::AscendingOrder);
        item1.setText(0, "b");
        item2.setText(0, "a");
        QCOMPARE(item1 < item2, true);
    }
}

QTEST_MAIN(tst_QTreeWidget)
#include "tst_qtreewidget.moc"
