/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qstandarditemmodel.h>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qstandarditem.h gui/itemviews/qstandarditem.cpp

class tst_QStandardItem : public QObject
{
    Q_OBJECT

public:
    tst_QStandardItem();
    virtual ~tst_QStandardItem();

public slots:
    void init();
    void cleanup();

private slots:
    void ctor();
    void textCtor();
    void iconTextCtor();
    void rowsColumnsCtor();
    void itemsCtor();
    void getSetData();
    void getSetFlags();
    void getSetRowAndColumnCount();
    void getSetChild_data();
    void getSetChild();
    void parent();
    void isTopLevelItem();
    void insertColumn_data();
    void insertColumn();
    void insertColumns_data();
    void insertColumns();
    void insertRow_data();
    void insertRow();
    void insertRows_data();
    void insertRows();
    void appendColumn_data();
    void appendColumn();
    void appendRow_data();
    void appendRow();
    void takeChild();
    void takeColumn_data();
    void takeColumn();
    void takeRow_data();
    void takeRow();
    void streamItem();
    void deleteItem();
    void clone();
};

tst_QStandardItem::tst_QStandardItem()
{
}

tst_QStandardItem::~tst_QStandardItem()
{
}

void tst_QStandardItem::init()
{
}

void tst_QStandardItem::cleanup()
{
}

void tst_QStandardItem::ctor()
{
    QStandardItem item;
    QVERIFY(!item.hasChildren());
}

void tst_QStandardItem::textCtor()
{
    QLatin1String text("text");
    QStandardItem item(text);
    QCOMPARE(item.text(), text);
    QVERIFY(!item.hasChildren());
}

void tst_QStandardItem::iconTextCtor()
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::red);
    QIcon icon(pixmap);
    QLatin1String text("text");
    QStandardItem item(icon, text);
    QCOMPARE(item.icon(), icon);
    QCOMPARE(item.text(), text);
    QVERIFY(!item.hasChildren());
}

void tst_QStandardItem::rowsColumnsCtor()
{
    const int rows = 5;
    const int columns = 12;
    QStandardItem item(rows, columns);
    QCOMPARE(item.rowCount(), rows);
    QCOMPARE(item.columnCount(), columns);
}

void tst_QStandardItem::itemsCtor()
{
    const int count = 10;
    QList<QStandardItem*> itemList;
    for (int i = 0; i < count; ++i)
        itemList.append(new QStandardItem(QString("%0").arg(i)));
    QStandardItem item(itemList);
    QCOMPARE(item.columnCount(), 1);
    QCOMPARE(item.rowCount(), count);
    for (int i = 0; i < count; ++i)
        QCOMPARE(item.child(i), itemList.at(i));
}

void tst_QStandardItem::getSetData()
{
    QStandardItem item;
    
    QLatin1String text("text");
    item.setText(text);
    QCOMPARE(item.text(), text);
    
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::red);
    QIcon icon(pixmap);
    item.setIcon(icon);
    QCOMPARE(item.icon(), icon);
    
    QLatin1String toolTip("toolTip");
    item.setToolTip(toolTip);
    QCOMPARE(item.toolTip(), toolTip);
    
    QLatin1String statusTip("statusTip");
    item.setStatusTip(statusTip);
    QCOMPARE(item.statusTip(), statusTip);

    QLatin1String whatsThis("whatsThis");
    item.setWhatsThis(whatsThis);
    QCOMPARE(item.whatsThis(), whatsThis);

    QSize sizeHint(64, 48);
    item.setSizeHint(sizeHint);
    QCOMPARE(item.sizeHint(), sizeHint);

    QFont font;
    item.setFont(font);
    QCOMPARE(item.font(), font);

    Qt::Alignment textAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    item.setTextAlignment(textAlignment);
    QCOMPARE(item.textAlignment(), textAlignment);

    QColor backgroundColor(Qt::blue);
    item.setBackgroundColor(backgroundColor);
    QCOMPARE(item.backgroundColor(), backgroundColor);
    
    QColor textColor(Qt::green);
    item.setTextColor(textColor);
    QCOMPARE(item.textColor(), textColor);

    Qt::CheckState checkState(Qt::PartiallyChecked);
    item.setCheckState(checkState);
    QCOMPARE(item.checkState(), checkState);

    QLatin1String accessibleText("accessibleText");
    item.setAccessibleText(accessibleText);
    QCOMPARE(item.accessibleText(), accessibleText);

    QLatin1String accessibleDescription("accessibleDescription");
    item.setAccessibleDescription(accessibleDescription);
    QCOMPARE(item.accessibleDescription(), accessibleDescription);

    QCOMPARE(item.text(), text);
    QCOMPARE(item.icon(), icon);
    QCOMPARE(item.toolTip(), toolTip);
    QCOMPARE(item.statusTip(), statusTip);
    QCOMPARE(item.whatsThis(), whatsThis);
    QCOMPARE(item.sizeHint(), sizeHint);
    QCOMPARE(item.font(), font);
    QCOMPARE(item.textAlignment(), textAlignment);
    QCOMPARE(item.backgroundColor(), backgroundColor);
    QCOMPARE(item.textColor(), textColor);
    QCOMPARE(item.checkState(), checkState);
    QCOMPARE(item.accessibleText(), accessibleText);
    QCOMPARE(item.accessibleDescription(), accessibleDescription);

    QCOMPARE(qvariant_cast<QString>(item.data(Qt::DisplayRole)), text);
    QCOMPARE(qvariant_cast<QIcon>(item.data(Qt::DecorationRole)), icon);
    QCOMPARE(qvariant_cast<QString>(item.data(Qt::ToolTipRole)), toolTip);
    QCOMPARE(qvariant_cast<QString>(item.data(Qt::StatusTipRole)), statusTip);
    QCOMPARE(qvariant_cast<QString>(item.data(Qt::WhatsThisRole)), whatsThis);
    QCOMPARE(qvariant_cast<QSize>(item.data(Qt::SizeHintRole)), sizeHint);
    QCOMPARE(qvariant_cast<QFont>(item.data(Qt::FontRole)), font);
    QCOMPARE(qvariant_cast<int>(item.data(Qt::TextAlignmentRole)), int(textAlignment));
    QCOMPARE(qvariant_cast<QColor>(item.data(Qt::BackgroundColorRole)), backgroundColor);
    QCOMPARE(qvariant_cast<QColor>(item.data(Qt::TextColorRole)), textColor);
    QCOMPARE(qvariant_cast<int>(item.data(Qt::CheckStateRole)), int(checkState));
    QCOMPARE(qvariant_cast<QString>(item.data(Qt::AccessibleTextRole)), accessibleText);
    QCOMPARE(qvariant_cast<QString>(item.data(Qt::AccessibleDescriptionRole)), accessibleDescription);
}

void tst_QStandardItem::getSetFlags()
{
    QStandardItem item;
    item.setEnabled(true);
    QVERIFY(item.isEnabled());
    QVERIFY(item.flags() & Qt::ItemIsEnabled);
    item.setEditable(true);
    QVERIFY(item.isEditable());
    QVERIFY(item.flags() & Qt::ItemIsEditable);
    item.setSelectable(true);
    QVERIFY(item.isSelectable());
    QVERIFY(item.flags() & Qt::ItemIsSelectable);
    item.setCheckable(true);
    QVERIFY(item.isCheckable());
    QVERIFY(item.flags() & Qt::ItemIsUserCheckable);
    item.setTristate(true);
    QVERIFY(item.isTristate());
    QVERIFY(item.flags() & Qt::ItemIsTristate);
    item.setDragEnabled(true);
    QVERIFY(item.isDragEnabled());
    QVERIFY(item.flags() & Qt::ItemIsDragEnabled);
    item.setDropEnabled(true);
    QVERIFY(item.isDropEnabled());
    QVERIFY(item.flags() & Qt::ItemIsDropEnabled);
    
    QVERIFY(item.isEnabled());
    item.setEnabled(false);
    QVERIFY(!item.isEnabled());
    QVERIFY(!(item.flags() & Qt::ItemIsEnabled));
    QVERIFY(item.isEditable());
    item.setEditable(false);
    QVERIFY(!item.isEditable());
    QVERIFY(!(item.flags() & Qt::ItemIsEditable));
    QVERIFY(item.isSelectable());
    item.setSelectable(false);
    QVERIFY(!item.isSelectable());
    QVERIFY(!(item.flags() & Qt::ItemIsSelectable));
    QVERIFY(item.isCheckable());
    item.setCheckable(false);
    QVERIFY(!item.isCheckable());
    QVERIFY(!(item.flags() & Qt::ItemIsUserCheckable));
    QVERIFY(item.isTristate());
    item.setTristate(false);
    QVERIFY(!item.isTristate());
    QVERIFY(!(item.flags() & Qt::ItemIsTristate));
    QVERIFY(item.isDragEnabled());
    item.setDragEnabled(false);
    QVERIFY(!item.isDragEnabled());
    QVERIFY(!(item.flags() & Qt::ItemIsDragEnabled));
    QVERIFY(item.isDropEnabled());
    item.setDropEnabled(false);
    QVERIFY(!item.isDropEnabled());
    QVERIFY(!(item.flags() & Qt::ItemIsDropEnabled));
}    

void tst_QStandardItem::getSetRowAndColumnCount()
{
    QStandardItem item;

    item.setRowCount(-1);
    QCOMPARE(item.rowCount(), 0);

    item.setColumnCount(-1);
    QCOMPARE(item.columnCount(), 0);

    item.setRowCount(1);
    QCOMPARE(item.rowCount(), 1);
    QCOMPARE(item.columnCount(), 0);

    item.setColumnCount(1);
    QCOMPARE(item.columnCount(), 1);
    QCOMPARE(item.rowCount(), 1);

    item.setColumnCount(10);
    QCOMPARE(item.columnCount(), 10);
    QCOMPARE(item.rowCount(), 1);

    item.setRowCount(20);
    QCOMPARE(item.rowCount(), 20);
    QCOMPARE(item.columnCount(), 10);

    item.setRowCount(-1);
    QCOMPARE(item.rowCount(), 20);

    item.setColumnCount(-1);
    QCOMPARE(item.columnCount(), 10);

    item.setColumnCount(0);
    QCOMPARE(item.columnCount(), 0);
    QCOMPARE(item.rowCount(), 20);

    item.setRowCount(0);
    QCOMPARE(item.rowCount(), 0);
}

void tst_QStandardItem::getSetChild_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");

    QTest::newRow("0x0 children, child at (-1,-1)") << 0 << 0 << -1 << -1;
    QTest::newRow("0x0 children, child at (0,0)") << 0 << 0 << 0 << 0;
}

void tst_QStandardItem::getSetChild()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(int, row);
    QFETCH(int, column);

    QStandardItem item(rows, columns);
    bool shouldHaveChildren = (rows > 0) && (columns > 0);
    QCOMPARE(item.hasChildren(), shouldHaveChildren);
    QCOMPARE(item.child(row, column), static_cast<QStandardItem*>(0));

    QStandardItem *child = new QStandardItem;
    item.setChild(row, column, child);
    if ((row >= 0) && (column >= 0)) {
        QCOMPARE(item.rowCount(), qMax(rows, row + 1));
        QCOMPARE(item.columnCount(), qMax(columns, column + 1));

        QCOMPARE(item.child(row, column), child);
        QCOMPARE(child->row(), row);
        QCOMPARE(child->column(), column);

        QStandardItem *anotherChild = new QStandardItem;
        item.setChild(row, column, anotherChild);
        QCOMPARE(item.child(row, column), anotherChild);
        QCOMPARE(anotherChild->row(), row);
        QCOMPARE(anotherChild->column(), column);
        item.setChild(row, column, 0);
    } else {
        delete child;
    }
    QCOMPARE(item.child(row, column), static_cast<QStandardItem*>(0));
}

void tst_QStandardItem::parent()
{
    QStandardItem item;
    QStandardItem *child = new QStandardItem;
    QCOMPARE(child->parent(), static_cast<QStandardItem*>(0));
    item.setChild(0, 0, child);
    QCOMPARE(child->parent(), &item);

    QStandardItem *childOfChild = new QStandardItem;
    child->setChild(0, 0, childOfChild);
    QCOMPARE(childOfChild->parent(), child);
}

void tst_QStandardItem::isTopLevelItem()
{
    QStandardItemModel model;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            QStandardItem *item = new QStandardItem;
            model.setItem(i, j, item);
            QVERIFY(item->isTopLevelItem());
            for (int k = 0; k < 10; ++k) {
                QStandardItem *child = new QStandardItem;
                item->setChild(k, child);
                QVERIFY(!child->isTopLevelItem());
            }
        }
    }
}

void tst_QStandardItem::insertColumn_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("count");

    QTest::newRow("insert 0 at -1 in 0x0") << 0 << 0 << -1 << 0;
    QTest::newRow("insert 0 at 0 in 0x0") << 0 << 0 << 0 << 0;
    QTest::newRow("insert 0 at 0 in 1x0") << 1 << 0 << 0 << 0;
    QTest::newRow("insert 0 at 0 in 0x1") << 0 << 1 << 0 << 0;
    QTest::newRow("insert 0 at 0 in 1x1") << 1 << 1 << 0 << 0;
    QTest::newRow("insert 1 at -1 in 0x0") << 0 << 0 << -1 << 1;
    QTest::newRow("insert 1 at 0 in 0x0") << 0 << 0 << 0 << 1;
    QTest::newRow("insert 1 at 0 in 1x0") << 1 << 0 << 0 << 1;
    QTest::newRow("insert 1 at 0 in 0x1") << 0 << 1 << 0 << 1;
    QTest::newRow("insert 1 at 0 in 1x1") << 1 << 1 << 0 << 1;
    QTest::newRow("insert 1 at 1 in 1x1") << 1 << 1 << 1 << 1;
    QTest::newRow("insert 1 at 0 in 2x1") << 2 << 1 << 0 << 1;
    QTest::newRow("insert 1 at 1 in 2x1") << 2 << 1 << 1 << 1;
    QTest::newRow("insert 1 at 0 in 1x2") << 1 << 2 << 0 << 1;
    QTest::newRow("insert 1 at 1 in 1x2") << 1 << 2 << 1 << 1;
    QTest::newRow("insert 1 at 0 in 8x4") << 8 << 4 << 0 << 1;
    QTest::newRow("insert 1 at 1 in 8x4") << 8 << 4 << 1 << 1;
    QTest::newRow("insert 1 at 2 in 8x4") << 8 << 4 << 2 << 1;
    QTest::newRow("insert 1 at 3 in 8x4") << 8 << 4 << 3 << 1;
    QTest::newRow("insert 1 at 4 in 8x4") << 8 << 4 << 4 << 1;
    QTest::newRow("insert 4 at 0 in 8x4") << 8 << 4 << 0 << 4;
    QTest::newRow("insert 4 at 4 in 8x4") << 8 << 4 << 4 << 4;
    QTest::newRow("insert 6 at 0 in 8x4") << 8 << 4 << 0 << 6;
    QTest::newRow("insert 6 at 4 in 8x4") << 8 << 4 << 4 << 6;
}

void tst_QStandardItem::insertColumn()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(int, column);
    QFETCH(int, count);

    QStandardItem item(rows, columns);

    // make items for a new column
    QList<QStandardItem*> columnItems;
    for (int i = 0; i < count; ++i)
        columnItems.append(new QStandardItem);

    item.insertColumn(column, columnItems);

    if (column >= 0) {
        QCOMPARE(item.columnCount(), columns + 1);
        QCOMPARE(item.rowCount(), qMax(rows, count));
        // check to make sure items were inserted in correct place
        for (int i = 0; i < count; ++i)
            QCOMPARE(item.child(i, column), columnItems.at(i));
        for (int i = count; i < item.rowCount(); ++i)
            QCOMPARE(item.child(i, column), static_cast<QStandardItem*>(0));
    } else {
        QCOMPARE(item.columnCount(), columns);
        QCOMPARE(item.rowCount(), rows);
        qDeleteAll(columnItems);
    }
}

void tst_QStandardItem::insertColumns_data()
{
}

void tst_QStandardItem::insertColumns()
{
}

void tst_QStandardItem::insertRow_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("count");

    QTest::newRow("insert 0 at -1 in 0x0") << 0 << 0 << -1 << 0;
    QTest::newRow("insert 0 at 0 in 0x0") << 0 << 0 << 0 << 0;
    QTest::newRow("insert 0 at 0 in 1x0") << 1 << 0 << 0 << 0;
    QTest::newRow("insert 0 at 0 in 0x1") << 0 << 1 << 0 << 0;
    QTest::newRow("insert 0 at 0 in 1x1") << 1 << 1 << 0 << 0;
    QTest::newRow("insert 1 at -1 in 0x0") << 0 << 0 << -1 << 1;
    QTest::newRow("insert 1 at 0 in 0x0") << 0 << 0 << 0 << 1;
    QTest::newRow("insert 1 at 0 in 1x0") << 1 << 0 << 0 << 1;
    QTest::newRow("insert 1 at 0 in 0x1") << 0 << 1 << 0 << 1;
    QTest::newRow("insert 1 at 0 in 1x1") << 1 << 1 << 0 << 1;
    QTest::newRow("insert 1 at 1 in 1x1") << 1 << 1 << 1 << 1;
    QTest::newRow("insert 1 at 0 in 2x1") << 2 << 1 << 0 << 1;
    QTest::newRow("insert 1 at 1 in 2x1") << 2 << 1 << 1 << 1;
    QTest::newRow("insert 1 at 0 in 1x2") << 1 << 2 << 0 << 1;
    QTest::newRow("insert 1 at 1 in 1x2") << 1 << 2 << 1 << 1;
    QTest::newRow("insert 1 at 0 in 4x8") << 4 << 8 << 0 << 1;
    QTest::newRow("insert 1 at 1 in 4x8") << 4 << 8 << 1 << 1;
    QTest::newRow("insert 1 at 2 in 4x8") << 4 << 8 << 2 << 1;
    QTest::newRow("insert 1 at 3 in 4x8") << 4 << 8 << 3 << 1;
    QTest::newRow("insert 1 at 4 in 4x8") << 4 << 8 << 4 << 1;
    QTest::newRow("insert 4 at 0 in 4x8") << 4 << 8 << 0 << 4;
    QTest::newRow("insert 4 at 4 in 4x8") << 4 << 8 << 4 << 4;
    QTest::newRow("insert 6 at 0 in 4x8") << 4 << 8 << 0 << 6;
    QTest::newRow("insert 6 at 4 in 4x8") << 4 << 8 << 4 << 6;
}

void tst_QStandardItem::insertRow()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(int, row);
    QFETCH(int, count);

    QStandardItem item(rows, columns);

    // make items for a new column
    QList<QStandardItem*> rowItems;
    for (int i = 0; i < count; ++i)
        rowItems.append(new QStandardItem);

    item.insertRow(row, rowItems);

    if (row >= 0) {
        QCOMPARE(item.columnCount(), qMax(columns, count));
        QCOMPARE(item.rowCount(), rows + 1);
        // check to make sure items were inserted in correct place
        for (int i = 0; i < count; ++i)
            QCOMPARE(item.child(row, i), rowItems.at(i));
        for (int i = count; i < item.columnCount(); ++i)
            QCOMPARE(item.child(row, i), static_cast<QStandardItem*>(0));
    } else {
        QCOMPARE(item.columnCount(), columns);
        QCOMPARE(item.rowCount(), rows);
        qDeleteAll(rowItems);
    }
}

void tst_QStandardItem::insertRows_data()
{
}

void tst_QStandardItem::insertRows()
{
}

void tst_QStandardItem::appendColumn_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<int>("count");

    QTest::newRow("append 0 to 0x0") << 0 << 0 << 0;
    QTest::newRow("append 1 to 0x0") << 0 << 0 << 1;
    QTest::newRow("append 1 to 1x0") << 1 << 0 << 1;
    QTest::newRow("append 1 to 0x1") << 0 << 1 << 1;
    QTest::newRow("append 1 to 1x1") << 1 << 1 << 1;
    QTest::newRow("append 1 to 2x0") << 2 << 0 << 1;
    QTest::newRow("append 1 to 0x2") << 0 << 2 << 1;
    QTest::newRow("append 1 to 2x1") << 2 << 1 << 1;
    QTest::newRow("append 1 to 1x2") << 1 << 2 << 1;
    QTest::newRow("append 1 to 2x2") << 2 << 2 << 1;
    QTest::newRow("append 2 to 0x0") << 0 << 0 << 2;
    QTest::newRow("append 2 to 1x0") << 1 << 0 << 2;
    QTest::newRow("append 2 to 0x1") << 0 << 1 << 2;
    QTest::newRow("append 2 to 1x1") << 1 << 1 << 2;
    QTest::newRow("append 2 to 2x0") << 2 << 0 << 2;
    QTest::newRow("append 2 to 0x2") << 0 << 2 << 2;
    QTest::newRow("append 2 to 2x1") << 2 << 1 << 2;
    QTest::newRow("append 2 to 1x2") << 1 << 2 << 2;
    QTest::newRow("append 2 to 2x2") << 2 << 2 << 2;
    QTest::newRow("append 3 to 2x1") << 2 << 1 << 3;
    QTest::newRow("append 3 to 1x2") << 1 << 2 << 3;
    QTest::newRow("append 3 to 2x2") << 2 << 2 << 3;
    QTest::newRow("append 3 to 4x2") << 4 << 2 << 3;
    QTest::newRow("append 3 to 2x4") << 2 << 4 << 3;
    QTest::newRow("append 3 to 4x4") << 4 << 4 << 3;
    QTest::newRow("append 7 to 4x2") << 4 << 2 << 7;
    QTest::newRow("append 7 to 2x4") << 2 << 4 << 7;
    QTest::newRow("append 7 to 4x4") << 4 << 4 << 7;
}

void tst_QStandardItem::appendColumn()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(int, count);

    QStandardItem item(rows, columns);
    QList<QStandardItem*> originalChildren;
    // initialize children
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QStandardItem *child = new QStandardItem;
            originalChildren.append(child);
            item.setChild(i, j, child);
        }
    }

    // make items for a new column
    QList<QStandardItem*> columnItems;
    for (int i = 0; i < count; ++i)
        columnItems.append(new QStandardItem);

    item.appendColumn(columnItems);

    QCOMPARE(item.columnCount(), columns + 1);
    QCOMPARE(item.rowCount(), qMax(rows, count));
    // check to make sure items were inserted in correct place
    for (int i = 0; i < count; ++i)
        QCOMPARE(item.child(i, columns), columnItems.at(i));
    for (int i = count; i < item.rowCount(); ++i)
        QCOMPARE(item.child(i, columns), static_cast<QStandardItem*>(0));

    // make sure original children remained unchanged
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j)
            QCOMPARE(item.child(i, j), originalChildren.at(i*columns+j));
    }
}

void tst_QStandardItem::appendRow_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<int>("count");

    QTest::newRow("append 0 to 0x0") << 0 << 0 << 0;
    QTest::newRow("append 1 to 0x0") << 0 << 0 << 1;
    QTest::newRow("append 1 to 1x0") << 1 << 0 << 1;
    QTest::newRow("append 1 to 0x1") << 0 << 1 << 1;
    QTest::newRow("append 1 to 1x1") << 1 << 1 << 1;
    QTest::newRow("append 1 to 2x0") << 2 << 0 << 1;
    QTest::newRow("append 1 to 0x2") << 0 << 2 << 1;
    QTest::newRow("append 1 to 2x1") << 2 << 1 << 1;
    QTest::newRow("append 1 to 1x2") << 1 << 2 << 1;
    QTest::newRow("append 1 to 2x2") << 2 << 2 << 1;
    QTest::newRow("append 2 to 0x0") << 0 << 0 << 2;
    QTest::newRow("append 2 to 1x0") << 1 << 0 << 2;
    QTest::newRow("append 2 to 0x1") << 0 << 1 << 2;
    QTest::newRow("append 2 to 1x1") << 1 << 1 << 2;
    QTest::newRow("append 2 to 2x0") << 2 << 0 << 2;
    QTest::newRow("append 2 to 0x2") << 0 << 2 << 2;
    QTest::newRow("append 2 to 2x1") << 2 << 1 << 2;
    QTest::newRow("append 2 to 1x2") << 1 << 2 << 2;
    QTest::newRow("append 2 to 2x2") << 2 << 2 << 2;
    QTest::newRow("append 3 to 2x1") << 2 << 1 << 3;
    QTest::newRow("append 3 to 1x2") << 1 << 2 << 3;
    QTest::newRow("append 3 to 2x2") << 2 << 2 << 3;
    QTest::newRow("append 3 to 4x2") << 4 << 2 << 3;
    QTest::newRow("append 3 to 2x4") << 2 << 4 << 3;
    QTest::newRow("append 3 to 4x4") << 4 << 4 << 3;
    QTest::newRow("append 7 to 4x2") << 4 << 2 << 7;
    QTest::newRow("append 7 to 2x4") << 2 << 4 << 7;
    QTest::newRow("append 7 to 4x4") << 4 << 4 << 7;
}

void tst_QStandardItem::appendRow()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(int, count);

    QStandardItem item(rows, columns);
    QList<QStandardItem*> originalChildren;
    // initialize children
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QStandardItem *child = new QStandardItem;
            originalChildren.append(child);
            item.setChild(i, j, child);
        }
    }

    // make items for a new row
    QList<QStandardItem*> rowItems;
    for (int i = 0; i < count; ++i)
        rowItems.append(new QStandardItem);

    item.appendRow(rowItems);

    QCOMPARE(item.rowCount(), rows + 1);
    QCOMPARE(item.columnCount(), qMax(columns, count));
    // check to make sure items were inserted in correct place
    for (int i = 0; i < count; ++i)
        QCOMPARE(item.child(rows, i), rowItems.at(i));
    for (int i = count; i < item.columnCount(); ++i)
        QCOMPARE(item.child(rows, i), static_cast<QStandardItem*>(0));

    // make sure original children remained unchanged
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j)
            QCOMPARE(item.child(i, j), originalChildren.at(i*columns+j));
    }
}

void tst_QStandardItem::takeChild()
{
    QList<QStandardItem*> itemList;
    for (int i = 0; i < 10; ++i)
        itemList.append(new QStandardItem);
    QStandardItem item(itemList);

    for (int i = 0; i < item.rowCount(); ++i) {
        QCOMPARE(item.takeChild(i), itemList.at(i));
        QCOMPARE(item.takeChild(0, 0), static_cast<QStandardItem*>(0));
        for (int j = i + 1; j < item.rowCount(); ++j)
            QCOMPARE(item.child(j), itemList.at(j));
    }
    qDeleteAll(itemList);
}

void tst_QStandardItem::takeColumn_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<int>("column");
    QTest::addColumn<bool>("expectSuccess");

    QTest::newRow("take -1 from 0x0") << 0 << 0 << -1 << false;
    QTest::newRow("take 0 from 0x0") << 0 << 0 << 0 << false;
    QTest::newRow("take 0 from 1x0") << 1 << 0 << 0 << false;
    QTest::newRow("take 0 from 0x1") << 0 << 1 << 0 << true;
    QTest::newRow("take 1 from 0x1") << 0 << 1 << 1 << false;
    QTest::newRow("take 0 from 1x1") << 1 << 1 << 0 << true;
    QTest::newRow("take 1 from 1x1") << 0 << 1 << 1 << false;
    QTest::newRow("take 0 from 4x1") << 4 << 1 << 0 << true;
    QTest::newRow("take 1 from 4x1") << 4 << 1 << 1 << false;
    QTest::newRow("take 0 from 4x8") << 4 << 8 << 0 << true;
    QTest::newRow("take 7 from 4x8") << 4 << 8 << 7 << true;
    QTest::newRow("take 8 from 4x8") << 4 << 8 << 8 << false;
}

void tst_QStandardItem::takeColumn()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(int, column);
    QFETCH(bool, expectSuccess);

    QStandardItem item(rows, columns);
    QList<QStandardItem*> originalChildren;
    // initialize children
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QStandardItem *child = new QStandardItem;
            originalChildren.append(child);
            item.setChild(i, j, child);
        }
    }

    QList<QStandardItem *> taken = item.takeColumn(column);
    if (expectSuccess) {
        QCOMPARE(taken.count(), item.rowCount());
        QCOMPARE(item.columnCount(), columns - 1);
        int index = column;
        for (int i = 0; i < taken.count(); ++i) {
            QCOMPARE(taken.at(i), originalChildren.takeAt(index));
            index += item.columnCount();
        }
        index = 0;
        for (int i = 0; i < item.rowCount(); ++i) {
            for (int j = 0; j < item.columnCount(); ++j) {
                QCOMPARE(item.child(i, j), originalChildren.at(index));
                ++index;
            }
        }
    } else {
        QVERIFY(taken.isEmpty());
    }
    qDeleteAll(taken);
}

void tst_QStandardItem::takeRow_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<int>("row");
    QTest::addColumn<bool>("expectSuccess");

    QTest::newRow("take -1 from 0x0") << 0 << 0 << -1 << false;
    QTest::newRow("take 0 from 0x0") << 0 << 0 << 0 << false;
    QTest::newRow("take 0 from 1x0") << 1 << 0 << 0 << true;
    QTest::newRow("take 0 from 0x1") << 0 << 1 << 0 << false;
    QTest::newRow("take 1 from 0x1") << 0 << 1 << 1 << false;
    QTest::newRow("take 0 from 1x1") << 1 << 1 << 0 << true;
    QTest::newRow("take 1 from 1x1") << 0 << 1 << 1 << false;
    QTest::newRow("take 0 from 1x4") << 1 << 4 << 0 << true;
    QTest::newRow("take 1 from 1x4") << 1 << 4 << 1 << false;
    QTest::newRow("take 0 from 8x4") << 8 << 4 << 0 << true;
    QTest::newRow("take 7 from 8x4") << 8 << 4 << 7 << true;
    QTest::newRow("take 8 from 8x4") << 8 << 4 << 8 << false;
}

void tst_QStandardItem::takeRow()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(int, row);
    QFETCH(bool, expectSuccess);

    QStandardItem item(rows, columns);
    QList<QStandardItem*> originalChildren;
    // initialize children
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QStandardItem *child = new QStandardItem;
            originalChildren.append(child);
            item.setChild(i, j, child);
        }
    }

    QList<QStandardItem *> taken = item.takeRow(row);
    if (expectSuccess) {
        QCOMPARE(taken.count(), item.columnCount());
        QCOMPARE(item.rowCount(), rows - 1);
        int index = row * columns;
        for (int i = 0; i < taken.count(); ++i) {
            QCOMPARE(taken.at(i), originalChildren.takeAt(index));
        }
        index = 0;
        for (int i = 0; i < item.rowCount(); ++i) {
            for (int j = 0; j < item.columnCount(); ++j) {
                QCOMPARE(item.child(i, j), originalChildren.at(index));
                ++index;
            }
        }
    } else {
        QVERIFY(taken.isEmpty());
    }
    qDeleteAll(taken);
}

void tst_QStandardItem::streamItem()
{
    QStandardItem item;
    
    item.setText(QLatin1String("text"));
    item.setToolTip(QLatin1String("toolTip"));
    item.setStatusTip(QLatin1String("statusTip"));
    item.setWhatsThis(QLatin1String("whatsThis"));
    item.setSizeHint(QSize(64, 48));
    item.setFont(QFont());
    item.setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    item.setBackgroundColor(QColor(Qt::blue));
    item.setTextColor(QColor(Qt::green));
    item.setCheckState(Qt::PartiallyChecked);
    item.setAccessibleText(QLatin1String("accessibleText"));
    item.setAccessibleDescription(QLatin1String("accessibleDescription"));

    QByteArray ba;
    {
        QDataStream ds(&ba, QIODevice::WriteOnly);
        ds << item;
    }
    {
        QStandardItem streamedItem;
        QDataStream ds(&ba, QIODevice::ReadOnly);
        ds >> streamedItem;
        QCOMPARE(streamedItem.text(), item.text());
        QCOMPARE(streamedItem.toolTip(), item.toolTip());
        QCOMPARE(streamedItem.statusTip(), item.statusTip());
        QCOMPARE(streamedItem.whatsThis(), item.whatsThis());
        QCOMPARE(streamedItem.sizeHint(), item.sizeHint());
        QCOMPARE(streamedItem.font(), item.font());
        QCOMPARE(streamedItem.textAlignment(), item.textAlignment());
        QCOMPARE(streamedItem.backgroundColor(), item.backgroundColor());
        QCOMPARE(streamedItem.textColor(), item.textColor());
        QCOMPARE(streamedItem.checkState(), item.checkState());
        QCOMPARE(streamedItem.accessibleText(), item.accessibleText());
        QCOMPARE(streamedItem.accessibleDescription(), item.accessibleDescription());
        QCOMPARE(streamedItem.flags(), item.flags());
    }
}

void tst_QStandardItem::deleteItem()
{
    QStandardItemModel model(4, 6);
    // initialize items
    for (int i = 0; i < model.rowCount(); ++i) {
        for (int j = 0; j < model.columnCount(); ++j) {
            QStandardItem *item = new QStandardItem();
            model.setItem(i, j, item);
        }
    }
    // delete items
    for (int i = 0; i < model.rowCount(); ++i) {
        for (int j = 0; j < model.columnCount(); ++j) {
            QStandardItem *item = model.item(i, j);
            delete item;
            QCOMPARE(model.item(i, j), static_cast<QStandardItem*>(0));
        }
    }
}

void tst_QStandardItem::clone()
{
    QStandardItem item;
    item.setText(QLatin1String("text"));
    item.setToolTip(QLatin1String("toolTip"));
    item.setStatusTip(QLatin1String("statusTip"));
    item.setWhatsThis(QLatin1String("whatsThis"));
    item.setSizeHint(QSize(64, 48));
    item.setFont(QFont());
    item.setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    item.setBackgroundColor(QColor(Qt::blue));
    item.setTextColor(QColor(Qt::green));
    item.setCheckState(Qt::PartiallyChecked);
    item.setAccessibleText(QLatin1String("accessibleText"));
    item.setAccessibleDescription(QLatin1String("accessibleDescription"));
    item.setFlags(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);

    QStandardItem *clone = item.clone();
    QCOMPARE(clone->text(), item.text());
    QCOMPARE(clone->toolTip(), item.toolTip());
    QCOMPARE(clone->statusTip(), item.statusTip());
    QCOMPARE(clone->whatsThis(), item.whatsThis());
    QCOMPARE(clone->sizeHint(), item.sizeHint());
    QCOMPARE(clone->font(), item.font());
    QCOMPARE(clone->textAlignment(), item.textAlignment());
    QCOMPARE(clone->backgroundColor(), item.backgroundColor());
    QCOMPARE(clone->textColor(), item.textColor());
    QCOMPARE(clone->checkState(), item.checkState());
    QCOMPARE(clone->accessibleText(), item.accessibleText());
    QCOMPARE(clone->accessibleDescription(), item.accessibleDescription());
    QCOMPARE(clone->flags(), item.flags());
    QVERIFY(!(*clone < item));
    delete clone;
}

QTEST_MAIN(tst_QStandardItem)
#include "tst_qstandarditem.moc"
