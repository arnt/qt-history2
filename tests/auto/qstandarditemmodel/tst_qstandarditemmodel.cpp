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
//TESTED_FILES=gui/itemviews/qstandarditemmodel.h gui/itemviews/qstandarditemmodel.cpp

class tst_QStandardItemModel : public QObject
{
    Q_OBJECT

public:
    tst_QStandardItemModel();
    virtual ~tst_QStandardItemModel();

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
    void init();
    void cleanup();

protected slots:
    void checkAboutToBeRemoved();
    void checkRemoved();
    void updateRowAboutToBeRemoved();

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

private slots:
    void insertRow_data();
    void insertRow();
    void insertRows();
    void insertRowInHierarcy();
    void insertColumn_data();
    void insertColumn();
    void insertColumns();
    void removeRows();
    void removeColumns();
    void setHeaderData();
    void persistentIndexes();
    void removingPersistentIndexes();
    void updatingPersistentIndexes();

    void checkChildren();
    void data();
    void clear();
#if QT_VERSION >= 0x040200
    void sort_data();
    void sort();
    void findItems();
    void getSetHeaderItem();
    void indexFromItem();
    void itemFromIndex();
    void getSetItemPrototype();
    void getSetItemData();
    void setHeaderLabels_data();
    void setHeaderLabels();
    void itemDataChanged();
    void takeHeaderItem();
    void useCase1();
    void useCase2();
#endif

private:
    QAbstractItemModel *model;
    QPersistentModelIndex persistent;
    QVector<QModelIndex> rcParent;
    QVector<int> rcFirst;
    QVector<int> rcLast;
};

static const int defaultSize = 3;

Q_DECLARE_METATYPE(QModelIndex)
#if QT_VERSION >= 0x040200
Q_DECLARE_METATYPE(QStandardItem*)
#endif
Q_DECLARE_METATYPE(Qt::Orientation)

tst_QStandardItemModel::tst_QStandardItemModel() : model(0), rcParent(8), rcFirst(8,0), rcLast(8,0)
{
}

tst_QStandardItemModel::~tst_QStandardItemModel()
{
}

/*
  This test usually uses a model with a 3x3 table
  ---------------------------
  |  0,0  |  0,1    |  0,2  |
  ---------------------------
  |  1,0  |  1,1    |  1,2  |
  ---------------------------
  |  2,0  |  2,1    |  2,2  |
  ---------------------------
*/
void tst_QStandardItemModel::init()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
#if QT_VERSION >= 0x040200
    qRegisterMetaType<QStandardItem*>("QStandardItem*");
#endif
    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");

    model = new QStandardItemModel(defaultSize, defaultSize);
    connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
            this, SLOT(rowsAboutToBeInserted(QModelIndex, int, int)));
    connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(rowsInserted(QModelIndex, int, int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
            this, SLOT(rowsAboutToBeRemoved(QModelIndex, int, int)));
    connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)),
            this, SLOT(rowsRemoved(QModelIndex, int, int)));

    connect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex, int, int)),
            this, SLOT(columnsAboutToBeInserted(QModelIndex, int, int)));
    connect(model, SIGNAL(columnsInserted(QModelIndex, int, int)),
            this, SLOT(columnsInserted(QModelIndex, int, int)));
    connect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex, int, int)),
            this, SLOT(columnsAboutToBeRemoved(QModelIndex, int, int)));
    connect(model, SIGNAL(columnsRemoved(QModelIndex, int, int)),
            this, SLOT(columnsRemoved(QModelIndex, int, int)));

    rcFirst.fill(-1);
    rcLast.fill(-1);
}

void tst_QStandardItemModel::cleanup()
{
    disconnect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
               this, SLOT(rowsAboutToBeInserted(QModelIndex, int, int)));
    disconnect(model, SIGNAL(rowsInserted(QModelIndex, int, int)),
               this, SLOT(rowsInserted(QModelIndex, int, int)));
    disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
               this, SLOT(rowsAboutToBeRemoved(QModelIndex, int, int)));
    disconnect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)),
               this, SLOT(rowsRemoved(QModelIndex, int, int)));

    disconnect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex, int, int)),
               this, SLOT(columnsAboutToBeInserted(QModelIndex, int, int)));
    disconnect(model, SIGNAL(columnsInserted(QModelIndex, int, int)),
               this, SLOT(columnsInserted(QModelIndex, int, int)));
    disconnect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex, int, int)),
               this, SLOT(columnsAboutToBeRemoved(QModelIndex, int, int)));
    disconnect(model, SIGNAL(columnsRemoved(QModelIndex, int, int)),
               this, SLOT(columnsRemoved(QModelIndex, int, int)));
    delete model;
    model = 0;
}

void tst_QStandardItemModel::insertRow_data()
{
    QTest::addColumn<int>("insertRow");
    QTest::addColumn<int>("expectedRow");

    QTest::newRow("Insert less then 0") << -1 << 0;
    QTest::newRow("Insert at 0")  << 0 << 0;
    QTest::newRow("Insert beyond count")  << defaultSize+1 << defaultSize;
    QTest::newRow("Insert at count") << defaultSize << defaultSize;
    QTest::newRow("Insert in the middle") << 1 << 1;
}

void tst_QStandardItemModel::insertRow()
{
    QFETCH(int, insertRow);
    QFETCH(int, expectedRow);

    QIcon icon;
    // default all initial items to DisplayRole: "initalitem"
    for (int r=0; r < model->rowCount(); ++r) {
        for (int c=0; c < model->columnCount(); ++c) {
            model->setData(model->index(r,c), "initialitem", Qt::DisplayRole);
        }
    }

    // check that inserts changes rowCount
    QCOMPARE(model->rowCount(), defaultSize);
    model->insertRow(insertRow);
    if (insertRow >= 0 && insertRow <= defaultSize) {
        QCOMPARE(model->rowCount(), defaultSize + 1);

        // check that signals were emitted with correct info
        QCOMPARE(rcFirst[RowsAboutToBeInserted], expectedRow);
        QCOMPARE(rcLast[RowsAboutToBeInserted], expectedRow);
        QCOMPARE(rcFirst[RowsInserted], expectedRow);
        QCOMPARE(rcLast[RowsInserted], expectedRow);

        //check that the inserted item has different DisplayRole than initial items
        QVERIFY(model->data(model->index(expectedRow, 0), Qt::DisplayRole).toString() != "initialitem");
    } else {
        // We inserted something outside the bounds, do nothing
        QCOMPARE(model->rowCount(), defaultSize);
        QCOMPARE(rcFirst[RowsAboutToBeInserted], -1);
        QCOMPARE(rcLast[RowsAboutToBeInserted], -1);
        QCOMPARE(rcFirst[RowsInserted], -1);
        QCOMPARE(rcLast[RowsInserted], -1);
    }
}

void tst_QStandardItemModel::insertRows()
{
    int rowCount = model->rowCount();
    QCOMPARE(rowCount, defaultSize);

    // insert custom header label
    QString headerLabel = "custom";
    model->setHeaderData(0, Qt::Vertical, headerLabel);

    // insert one row
    model->insertRows(0, 1);
    QCOMPARE(model->rowCount(), rowCount + 1);
    rowCount = model->rowCount();

    // check header data has moved
    QCOMPARE(model->headerData(1, Qt::Vertical).toString(), headerLabel);

    // insert two rows
    model->insertRows(0, 2);
    QCOMPARE(model->rowCount(), rowCount + 2);

    // check header data has moved
    QCOMPARE(model->headerData(3, Qt::Vertical).toString(), headerLabel);
}

void tst_QStandardItemModel::insertRowInHierarcy()
{
    QVERIFY(model->insertRows(0, 1, QModelIndex()));
    QVERIFY(model->insertColumns(0, 1, QModelIndex()));
    QVERIFY(model->hasIndex(0, 0, QModelIndex()));

    QModelIndex parent = model->index(0, 0, QModelIndex());
    QVERIFY(parent.isValid());

    QVERIFY(model->insertRows(0, 1, parent));
    QVERIFY(model->insertColumns(0, 1, parent));
    QVERIFY(model->hasIndex(0, 0, parent));

    QModelIndex child = model->index(0, 0, parent);
    QVERIFY(child.isValid());
}

void tst_QStandardItemModel::insertColumn_data()
{
    QTest::addColumn<int>("insertColumn");
    QTest::addColumn<int>("expectedColumn");

    QTest::newRow("Insert less then 0") << -1 << 0;
    QTest::newRow("Insert at 0")  << 0 << 0;
    QTest::newRow("Insert beyond count")  << defaultSize+1 << defaultSize;
    QTest::newRow("Insert at count") << defaultSize << defaultSize;
    QTest::newRow("Insert in the middle") << 1 << 1;
}

void tst_QStandardItemModel::insertColumn()
{
    QFETCH(int, insertColumn);
    QFETCH(int, expectedColumn);

    // default all initial items to DisplayRole: "initalitem"
    for (int r=0; r < model->rowCount(); ++r) {
        for (int c=0; c < model->columnCount(); ++c) {
            model->setData(model->index(r,c), "initialitem", Qt::DisplayRole);
        }
    }

    // check that inserts changes columnCount
    QCOMPARE(model->columnCount(), defaultSize);
    model->insertColumn(insertColumn);
    if (insertColumn >= 0 && insertColumn <= defaultSize) {
        QCOMPARE(model->columnCount(), defaultSize +  1);
        // check that signals were emitted with correct info
        QCOMPARE(rcFirst[ColumnsAboutToBeInserted], expectedColumn);
        QCOMPARE(rcLast[ColumnsAboutToBeInserted], expectedColumn);
        QCOMPARE(rcFirst[ColumnsInserted], expectedColumn);
        QCOMPARE(rcLast[ColumnsInserted], expectedColumn);

        //check that the inserted item has different DisplayRole than initial items
        QVERIFY(model->data(model->index(0, expectedColumn), Qt::DisplayRole).toString() != "initialitem");
    } else {
        // We inserted something outside the bounds, do nothing
        QCOMPARE(model->columnCount(), defaultSize);
        QCOMPARE(rcFirst[ColumnsAboutToBeInserted], -1);
        QCOMPARE(rcLast[ColumnsAboutToBeInserted], -1);
        QCOMPARE(rcFirst[ColumnsInserted], -1);
        QCOMPARE(rcLast[ColumnsInserted], -1);
    }

}

void tst_QStandardItemModel::insertColumns()
{
    int columnCount = model->columnCount();
    QCOMPARE(columnCount, defaultSize);

    // insert custom header label
    QString headerLabel = "custom";
    model->setHeaderData(0, Qt::Horizontal, headerLabel);

    // insert one column
    model->insertColumns(0, 1);
    QCOMPARE(model->columnCount(), columnCount + 1);
    columnCount = model->columnCount();

    // check header data has moved
    QCOMPARE(model->headerData(1, Qt::Horizontal).toString(), headerLabel);

    // insert two columns
    model->insertColumns(0, 2);
    QCOMPARE(model->columnCount(), columnCount + 2);

    // check header data has moved
    QCOMPARE(model->headerData(3, Qt::Horizontal).toString(), headerLabel);
}

void tst_QStandardItemModel::removeRows()
{
    int rowCount = model->rowCount();
    QCOMPARE(rowCount, defaultSize);

    // insert custom header label
    QString headerLabel = "custom";
    model->setHeaderData(rowCount - 1, Qt::Vertical, headerLabel);

    // remove one row
    model->removeRows(0, 1);
    QCOMPARE(model->rowCount(), rowCount - 1);
    rowCount = model->rowCount();

    // check header data has moved
    QCOMPARE(model->headerData(rowCount - 1, Qt::Vertical).toString(), headerLabel);

    // remove two rows
    model->removeRows(0, 2);
    QCOMPARE(model->rowCount(), rowCount - 2);
}

void tst_QStandardItemModel::removeColumns()
{
    int columnCount = model->columnCount();
    QCOMPARE(columnCount, defaultSize);

    // insert custom header label
    QString headerLabel = "custom";
    model->setHeaderData(columnCount - 1, Qt::Horizontal, headerLabel);

    // remove one column
    model->removeColumns(0, 1);
    QCOMPARE(model->columnCount(), columnCount - 1);
    columnCount = model->columnCount();

    // check header data has moved
    QCOMPARE(model->headerData(columnCount - 1, Qt::Horizontal).toString(), headerLabel);

    // remove two columns
    model->removeColumns(0, 2);
    QCOMPARE(model->columnCount(), columnCount - 2);
}


void tst_QStandardItemModel::setHeaderData()
{
    for (int x = 0; x < 2; ++x) {
        bool vertical = (x == 0);
        int count = vertical ? model->rowCount() : model->columnCount();
        QCOMPARE(count, defaultSize);
        Qt::Orientation orient = vertical ? Qt::Vertical : Qt::Horizontal;
        
        // check default values are ok
        for (int i = 0; i < count; ++i)
            QCOMPARE(model->headerData(i, orient).toString(), QString::number(i + 1));
        
        QSignalSpy headerDataChangedSpy(
            model, SIGNAL(headerDataChanged(Qt::Orientation, int, int)));
        // insert custom values and check
        for (int i = 0; i < count; ++i) {
            QString customString = QString("custom") + QString::number(i);
            QCOMPARE(model->setHeaderData(i, orient, customString), true);
            QCOMPARE(headerDataChangedSpy.count(), 1);
            QVariantList args = headerDataChangedSpy.takeFirst();
            QCOMPARE(qvariant_cast<Qt::Orientation>(args.at(0)), orient);
            QCOMPARE(args.at(1).toInt(), i);
            QCOMPARE(args.at(2).toInt(), i);
            QCOMPARE(model->headerData(i, orient).toString(), customString);
        }
        
        //check read from invalid sections
        QVERIFY(!model->headerData(count, orient).isValid());
        QVERIFY(!model->headerData(-1, orient).isValid());
        //check write to invalid section
        QCOMPARE(model->setHeaderData(count, orient, "foo"), false);
        QCOMPARE(model->setHeaderData(-1, orient, "foo"), false);
        QVERIFY(!model->headerData(count, orient).isValid());
        QVERIFY(!model->headerData(-1, orient).isValid());
    }
}

void tst_QStandardItemModel::persistentIndexes()
{
    QCOMPARE(model->rowCount(), defaultSize);
    QCOMPARE(model->columnCount(), defaultSize);

    // create a persisten index at 0,0
    QPersistentModelIndex persistentIndex(model->index(0, 0));

    // verify it is ok and at the correct spot
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.row(), 0);
    QCOMPARE(persistentIndex.column(), 0);

    // insert row and check that the persisten index has moved
    QVERIFY(model->insertRow(0));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.row(), 1);
    QCOMPARE(persistentIndex.column(), 0);

    // insert row after the persisten index and see that it stays the same
    QVERIFY(model->insertRow(model->rowCount()));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.row(), 1);
    QCOMPARE(persistentIndex.column(), 0);

    // insert column and check that the persisten index has moved
    QVERIFY(model->insertColumn(0));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.row(), 1);
    QCOMPARE(persistentIndex.column(), 1);

    // insert column after the persisten index and see that it stays the same
    QVERIFY(model->insertColumn(model->columnCount()));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.row(), 1);
    QCOMPARE(persistentIndex.column(), 1);

    // removes a row beyond the persistent index and see it stays the same
    QVERIFY(model->removeRow(model->rowCount() - 1));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.row(), 1);
    QCOMPARE(persistentIndex.column(), 1);

    // removes a column beyond the persistent index and see it stays the same
    QVERIFY(model->removeColumn(model->columnCount() - 1));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.row(), 1);
    QCOMPARE(persistentIndex.column(), 1);

    // removes a row before the persistent index and see it moves the same
    QVERIFY(model->removeRow(0));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.row(), 0);
    QCOMPARE(persistentIndex.column(), 1);

    // removes a column before the persistent index and see it moves the same
    QVERIFY(model->removeColumn(0));
    QVERIFY(persistentIndex.isValid());
    QCOMPARE(persistentIndex.row(), 0);
    QCOMPARE(persistentIndex.column(), 0);

    // remove the row where the persistent index is, and see that it becomes invalid
    QVERIFY(model->removeRow(0));
    QVERIFY(!persistentIndex.isValid());

    // remove the row where the persistent index is, and see that it becomes invalid
    persistentIndex = model->index(0, 0);
    QVERIFY(persistentIndex.isValid());
    QVERIFY(model->removeColumn(0));
    QVERIFY(!persistentIndex.isValid());
}

void tst_QStandardItemModel::checkAboutToBeRemoved()
{
    QVERIFY(persistent.isValid());
}

void tst_QStandardItemModel::checkRemoved()
{
    QVERIFY(!persistent.isValid());
}

void tst_QStandardItemModel::removingPersistentIndexes()
{
    // add 10 rows and columns to model to make it big enough
    QVERIFY(model->insertRows(0, 10));
    QVERIFY(model->insertColumns(0, 10));

    QObject::connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                     this, SLOT(checkAboutToBeRemoved()));
    QObject::connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                     this, SLOT(checkRemoved()));
    QObject::connect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                     this, SLOT(checkAboutToBeRemoved()));
    QObject::connect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                     this, SLOT(checkRemoved()));


    // test removeRow
    // add child table 3x3 to parent index(0, 0)
    QVERIFY(model->insertRows(0, 3, model->index(0, 0)));
    QVERIFY(model->insertColumns(0, 3, model->index(0, 0)));

    // set child to persistent and delete parent row
    persistent = model->index(0, 0, model->index(0, 0));
    QVERIFY(persistent.isValid());
    QVERIFY(model->removeRow(0));

    // set persistent to index(0, 0) and remove that row
    persistent = model->index(0, 0);
    QVERIFY(persistent.isValid());
    QVERIFY(model->removeRow(0));


    // test removeColumn
    // add child table 3x3 to parent index (0, 0)
    QVERIFY(model->insertRows(0, 3, model->index(0, 0)));
    QVERIFY(model->insertColumns(0, 3, model->index(0, 0)));

    // set child to persistent and delete parent column
    persistent = model->index(0, 0, model->index(0, 0));
    QVERIFY(persistent.isValid());
    QVERIFY(model->removeColumn(0));

    // set persistent to index(0, 0) and remove that column
    persistent = model->index(0, 0);
    QVERIFY(persistent.isValid());
    QVERIFY(model->removeColumn(0));


    QObject::disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                        this, SLOT(checkAboutToBeRemoved()));
    QObject::disconnect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                        this, SLOT(checkRemoved()));
    QObject::disconnect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                        this, SLOT(checkAboutToBeRemoved()));
    QObject::disconnect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                        this, SLOT(checkRemoved()));
}

void tst_QStandardItemModel::updateRowAboutToBeRemoved()
{
    QModelIndex idx = model->index(0, 0);
    QVERIFY(idx.isValid());
    persistent = idx;
}

void tst_QStandardItemModel::updatingPersistentIndexes()
{
    QObject::connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                     this, SLOT(updateRowAboutToBeRemoved()));

    persistent = model->index(1, 0);
    QVERIFY(persistent.isValid());
    QVERIFY(model->removeRow(1));
    QVERIFY(persistent.isValid());
    QPersistentModelIndex tmp = model->index(0, 0);
    QCOMPARE(persistent, tmp);

    QObject::disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                        this, SLOT(updateRowAboutToBeRemoved()));
}

void tst_QStandardItemModel::modelChanged(ModelChanged change, const QModelIndex &parent,
                                          int first, int last)
{
    rcParent[change] = parent;
    rcFirst[change] = first;
    rcLast[change] = last;
}


void tst_QStandardItemModel::checkChildren()
{
    QStandardItemModel model(0, 0);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 0);
    QVERIFY(!model.hasChildren());

    QVERIFY(model.insertRows(0, 1));
    QVERIFY(!model.hasChildren());
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.columnCount(), 0);

    QVERIFY(model.insertColumns(0, 1));
    QVERIFY(model.hasChildren());
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.columnCount(), 1);

    QModelIndex idx = model.index(0, 0);
    QVERIFY(!model.hasChildren(idx));
    QCOMPARE(model.rowCount(idx), 0);
    QCOMPARE(model.columnCount(idx), 0);

    QVERIFY(model.insertRows(0, 1, idx));
    QVERIFY(!model.hasChildren(idx));
    QCOMPARE(model.rowCount(idx), 1);
    QCOMPARE(model.columnCount(idx), 0);

    QVERIFY(model.insertColumns(0, 1, idx));
    QVERIFY(model.hasChildren(idx));
    QCOMPARE(model.rowCount(idx), 1);
    QCOMPARE(model.columnCount(idx), 1);

    QModelIndex idx2 = model.index(0, 0, idx);
    QVERIFY(!model.hasChildren(idx2));
    QCOMPARE(model.rowCount(idx2), 0);
    QCOMPARE(model.columnCount(idx2), 0);

    QVERIFY(model.removeRows(0, 1, idx));
    QVERIFY(model.hasChildren());
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.columnCount(), 1);
    QVERIFY(!model.hasChildren(idx));
    QCOMPARE(model.rowCount(idx), 0);
    QCOMPARE(model.columnCount(idx), 1);

    QVERIFY(model.removeRows(0, 1));
    QVERIFY(!model.hasChildren());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 1);
}

void tst_QStandardItemModel::data()
{
    // bad args
    model->setData(QModelIndex(), "bla", Qt::DisplayRole);

    QIcon icon;
    for (int r=0; r < model->rowCount(); ++r) {
        for (int c=0; c < model->columnCount(); ++c) {
            model->setData(model->index(r,c), "initialitem", Qt::DisplayRole);
            model->setData(model->index(r,c), "tooltip", Qt::ToolTipRole);
            model->setData(model->index(r,c), icon, Qt::DecorationRole);
        }
    }

    QVERIFY(model->data(model->index(0, 0), Qt::DisplayRole).toString() == "initialitem");
    QVERIFY(model->data(model->index(0, 0), Qt::ToolTipRole).toString() == "tooltip");

}

void tst_QStandardItemModel::clear()
{
    QStandardItemModel model;
    model.insertColumns(0, 10);
    model.insertRows(0, 10);
    QCOMPARE(model.columnCount(), 10);
    QCOMPARE(model.rowCount(), 10);
    model.clear();
    QCOMPARE(model.index(0, 0), QModelIndex());
#if QT_VERSION >= 0x040103
    QCOMPARE(model.columnCount(), 0);
#endif
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.hasChildren(), false);
}

#if QT_VERSION >= 0x040200
void tst_QStandardItemModel::sort_data()
{
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<QStringList>("initial");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("flat descending") << static_cast<int>(Qt::DescendingOrder)
                                  << (QStringList()
                                      << "delta"
                                      << "yankee"
                                      << "bravo"
                                      << "lima"
                                      << "charlie"
                                      << "juliet"
                                      << "tango"
                                      << "hotel"
                                      << "uniform"
                                      << "alpha"
                                      << "echo"
                                      << "golf"
                                      << "quebec"
                                      << "foxtrot"
                                      << "india"
                                      << "romeo"
                                      << "november"
                                      << "oskar"
                                      << "zulu"
                                      << "kilo"
                                      << "whiskey"
                                      << "mike"
                                      << "papa"
                                      << "sierra"
                                      << "xray"
                                      << "viktor")
                                  << (QStringList()
                                      << "zulu"
                                      << "yankee"
                                      << "xray"
                                      << "whiskey"
                                      << "viktor"
                                      << "uniform"
                                      << "tango"
                                      << "sierra"
                                      << "romeo"
                                      << "quebec"
                                      << "papa"
                                      << "oskar"
                                      << "november"
                                      << "mike"
                                      << "lima"
                                      << "kilo"
                                      << "juliet"
                                      << "india"
                                      << "hotel"
                                      << "golf"
                                      << "foxtrot"
                                      << "echo"
                                      << "delta"
                                      << "charlie"
                                      << "bravo"
                                      << "alpha");
    QTest::newRow("flat ascending") <<  static_cast<int>(Qt::AscendingOrder)
                                 << (QStringList()
                                     << "delta"
                                     << "yankee"
                                     << "bravo"
                                     << "lima"
                                     << "charlie"
                                     << "juliet"
                                     << "tango"
                                     << "hotel"
                                     << "uniform"
                                     << "alpha"
                                     << "echo"
                                     << "golf"
                                     << "quebec"
                                     << "foxtrot"
                                     << "india"
                                     << "romeo"
                                     << "november"
                                     << "oskar"
                                     << "zulu"
                                     << "kilo"
                                     << "whiskey"
                                     << "mike"
                                     << "papa"
                                     << "sierra"
                                     << "xray"
                                     << "viktor")
                                 << (QStringList()
                                     << "alpha"
                                     << "bravo"
                                     << "charlie"
                                     << "delta"
                                     << "echo"
                                     << "foxtrot"
                                     << "golf"
                                     << "hotel"
                                     << "india"
                                     << "juliet"
                                     << "kilo"
                                     << "lima"
                                     << "mike"
                                     << "november"
                                     << "oskar"
                                     << "papa"
                                     << "quebec"
                                     << "romeo"
                                     << "sierra"
                                     << "tango"
                                     << "uniform"
                                     << "viktor"
                                     << "whiskey"
                                     << "xray"
                                     << "yankee"
                                     << "zulu");
    QStringList list;
    for (int i=1000; i < 2000; ++i)
        list.append(QString("Number: %1").arg(i));
    QTest::newRow("large set ascending") <<  static_cast<int>(Qt::AscendingOrder) << list << list;
}

void tst_QStandardItemModel::sort()
{
    QFETCH(int, sortOrder);
    QFETCH(QStringList, initial);
    QFETCH(QStringList, expected);
    // prepare model
    QStandardItemModel model;
    QVERIFY(model.insertRows(0, initial.count(), QModelIndex()));
    QCOMPARE(model.rowCount(QModelIndex()), initial.count());
    model.insertColumns(0, 1, QModelIndex());
    QCOMPARE(model.columnCount(QModelIndex()), 1);
    for (int row = 0; row < model.rowCount(QModelIndex()); ++row) {
        QModelIndex index = model.index(row, 0, QModelIndex());
        model.setData(index, initial.at(row), Qt::DisplayRole);
    }

    // sort
    model.sort(0, static_cast<Qt::SortOrder>(sortOrder));

    // make sure the model is sorted
    for (int row = 0; row < model.rowCount(QModelIndex()); ++row) {
        QModelIndex index = model.index(row, 0, QModelIndex());
        QCOMPARE(model.data(index, Qt::DisplayRole).toString(), expected.at(row));
    }
}

void tst_QStandardItemModel::findItems()
{
    QStandardItemModel model;
    model.appendRow(new QStandardItem(QLatin1String("foo")));
    model.appendRow(new QStandardItem(QLatin1String("bar")));
    model.item(1)->appendRow(new QStandardItem(QLatin1String("foo")));
    QList<QStandardItem*> matches;
    matches = model.findItems(QLatin1String("foo"), Qt::MatchExactly|Qt::MatchRecursive, 0);
    QCOMPARE(matches.count(), 2);
    matches = model.findItems(QLatin1String("foo"), Qt::MatchExactly, 0);
    QCOMPARE(matches.count(), 1);
    matches = model.findItems(QLatin1String("food"), Qt::MatchExactly|Qt::MatchRecursive, 0);
    QCOMPARE(matches.count(), 0);
    matches = model.findItems(QLatin1String("foo"), Qt::MatchExactly|Qt::MatchRecursive, -1);
    QCOMPARE(matches.count(), 0);
    matches = model.findItems(QLatin1String("foo"), Qt::MatchExactly|Qt::MatchRecursive, 1);
    QCOMPARE(matches.count(), 0);
}

void tst_QStandardItemModel::getSetHeaderItem()
{
    QStandardItemModel model;

    QCOMPARE(model.horizontalHeaderItem(0), static_cast<QStandardItem*>(0));
    QStandardItem *hheader = new QStandardItem();
    model.setHorizontalHeaderItem(0, hheader);
    QCOMPARE(model.columnCount(), 1);
    QCOMPARE(model.horizontalHeaderItem(0), hheader);
    model.setHorizontalHeaderItem(0, 0);
    QCOMPARE(model.horizontalHeaderItem(0), static_cast<QStandardItem*>(0));

    QCOMPARE(model.verticalHeaderItem(0), static_cast<QStandardItem*>(0));
    QStandardItem *vheader = new QStandardItem();
    model.setVerticalHeaderItem(0, vheader);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.verticalHeaderItem(0), vheader);
    model.setVerticalHeaderItem(0, 0);
    QCOMPARE(model.verticalHeaderItem(0), static_cast<QStandardItem*>(0));
}

void tst_QStandardItemModel::indexFromItem()
{
    QStandardItemModel model;

    QCOMPARE(model.indexFromItem(model.topLevelParent()), QModelIndex());

    QStandardItem *item = new QStandardItem;
    model.setItem(10, 20, item);
    QModelIndex itemIndex = model.indexFromItem(item);
    QVERIFY(itemIndex.isValid());
    QCOMPARE(itemIndex.row(), 10);
    QCOMPARE(itemIndex.column(), 20);
    QCOMPARE(itemIndex.parent(), QModelIndex());

    QStandardItem *child = new QStandardItem;
    item->setChild(4, 2, child);
    QModelIndex childIndex = model.indexFromItem(child);
    QVERIFY(childIndex.isValid());
    QCOMPARE(childIndex.row(), 4);
    QCOMPARE(childIndex.column(), 2);
    QCOMPARE(childIndex.parent(), itemIndex);

    QStandardItem *dummy = new QStandardItem;
    QModelIndex noSuchIndex = model.indexFromItem(dummy);
    QVERIFY(!noSuchIndex.isValid());
    delete dummy;

    noSuchIndex = model.indexFromItem(0);
    QVERIFY(!noSuchIndex.isValid());
}

void tst_QStandardItemModel::itemFromIndex()
{
    QStandardItemModel model;

    QCOMPARE(model.itemFromIndex(QModelIndex()), model.topLevelParent());

    QStandardItem *item = new QStandardItem;
    model.setItem(10, 20, item);
    QModelIndex itemIndex = model.index(10, 20, QModelIndex());
    QVERIFY(itemIndex.isValid());
    QCOMPARE(model.itemFromIndex(itemIndex), item);

    QStandardItem *child = new QStandardItem;
    item->setChild(4, 2, child);
    QModelIndex childIndex = model.index(4, 2, itemIndex);
    QVERIFY(childIndex.isValid());
    QCOMPARE(model.itemFromIndex(childIndex), child);

    QModelIndex noSuchIndex = model.index(99, 99, itemIndex);
    QVERIFY(!noSuchIndex.isValid());
}

class CustomItem : public QStandardItem
{
public:
    CustomItem() : QStandardItem() { }
    ~CustomItem() { }
    int type() const {
        return UserType;
    }
    QStandardItem *clone() const {
        return new CustomItem;
    }
};

void tst_QStandardItemModel::getSetItemPrototype()
{
    QStandardItemModel model;
    QCOMPARE(model.itemPrototype(), static_cast<QStandardItem*>(0));
    CustomItem *proto = new CustomItem;
    model.setItemPrototype(proto);
    QCOMPARE(model.itemPrototype(), proto);

    model.setRowCount(1);
    model.setColumnCount(1);
    QModelIndex index = model.index(0, 0, QModelIndex());
    model.setData(index, "foo");
    QStandardItem *item = model.itemFromIndex(index);
    QVERIFY(item != 0);
    QCOMPARE(item->type(), static_cast<int>(QStandardItem::UserType));
}

void tst_QStandardItemModel::getSetItemData()
{
    QMap<int, QVariant> roles;
    QLatin1String text("text");
    roles.insert(Qt::DisplayRole, text);
    QLatin1String statusTip("statusTip");
    roles.insert(Qt::StatusTipRole, statusTip);
    QLatin1String toolTip("toolTip");
    roles.insert(Qt::ToolTipRole, toolTip);
    QLatin1String whatsThis("whatsThis");
    roles.insert(Qt::WhatsThisRole, whatsThis);
    QSize sizeHint(64, 48);
    roles.insert(Qt::SizeHintRole, sizeHint);
    QFont font;
    roles.insert(Qt::FontRole, font);
    Qt::Alignment textAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    roles.insert(Qt::TextAlignmentRole, int(textAlignment));
    QColor backgroundColor(Qt::blue);
    roles.insert(Qt::BackgroundColorRole, backgroundColor);
    QColor textColor(Qt::green);
    roles.insert(Qt::TextColorRole, textColor);
    Qt::CheckState checkState(Qt::PartiallyChecked);
    roles.insert(Qt::CheckStateRole, int(checkState));
    QLatin1String accessibleText("accessibleText");
    roles.insert(Qt::AccessibleTextRole, accessibleText);
    QLatin1String accessibleDescription("accessibleDescription");
    roles.insert(Qt::AccessibleDescriptionRole, accessibleDescription);

    QStandardItemModel model;
    model.insertRows(0, 1);
    model.insertColumns(0, 1);
    QModelIndex idx = model.index(0, 0, QModelIndex());
    QVERIFY(model.setItemData(idx, roles));
    QCOMPARE(model.itemData(idx), roles);
}

void tst_QStandardItemModel::setHeaderLabels_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("columns");
    QTest::addColumn<int>("orientation");
    QTest::addColumn<QStringList>("labels");
    QTest::addColumn<QStringList>("expectedLabels");

    QTest::newRow("horizontal labels")
        << 1
        << 4
        << int(Qt::Horizontal)
        << (QStringList() << "a" << "b" << "c" << "d")
        << (QStringList() << "a" << "b" << "c" << "d");
    QTest::newRow("vertical labels")
        << 4
        << 1
        << int(Qt::Vertical)
        << (QStringList() << "a" << "b" << "c" << "d")
        << (QStringList() << "a" << "b" << "c" << "d");
    QTest::newRow("too few (horizontal)")
        << 1
        << 4
        << int(Qt::Horizontal)
        << (QStringList() << "a" << "b")
        << (QStringList() << "a" << "b" << "3" << "4");
    QTest::newRow("too few (vertical)")
        << 4
        << 1
        << int(Qt::Vertical)
        << (QStringList() << "a" << "b")
        << (QStringList() << "a" << "b" << "3" << "4");
    QTest::newRow("too many (horizontal)")
        << 1
        << 2
        << int(Qt::Horizontal)
        << (QStringList() << "a" << "b" << "c" << "d")
        << (QStringList() << "a" << "b" << "c" << "d");
    QTest::newRow("too many (vertical)")
        << 2
        << 1
        << int(Qt::Vertical)
        << (QStringList() << "a" << "b" << "c" << "d")
        << (QStringList() << "a" << "b" << "c" << "d");
}

void tst_QStandardItemModel::setHeaderLabels()
{
    QFETCH(int, rows);
    QFETCH(int, columns);
    QFETCH(int, orientation);
    QFETCH(QStringList, labels);
    QFETCH(QStringList, expectedLabels);
    QStandardItemModel model(rows, columns);
    if (orientation == Qt::Horizontal)
        model.setHorizontalHeaderLabels(labels);
    else
        model.setVerticalHeaderLabels(labels);
    for (int i = 0; i < expectedLabels.count(); ++i)
        QCOMPARE(model.headerData(i, Qt::Orientation(orientation)).toString(), expectedLabels.at(i));
}

void tst_QStandardItemModel::itemDataChanged()
{
    QStandardItemModel model(6, 4);
    QStandardItem item;
    QSignalSpy dataChangedSpy(
        &model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)));
    QSignalSpy itemChangedSpy(
        &model, SIGNAL(itemChanged(QStandardItem *)));

    model.setItem(0, &item);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(itemChangedSpy.count(), 1);
    QModelIndex index = model.indexFromItem(&item);
    QList<QVariant> args;
    args = dataChangedSpy.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), index);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(1)), index);
    args = itemChangedSpy.takeFirst();
    QCOMPARE(qvariant_cast<QStandardItem*>(args.at(0)), &item);

    item.setData(Qt::DisplayRole, QLatin1String("foo"));
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(itemChangedSpy.count(), 1);
    args = dataChangedSpy.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), index);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(1)), index);
    args = itemChangedSpy.takeFirst();
    QCOMPARE(qvariant_cast<QStandardItem*>(args.at(0)), &item);

    item.setData(Qt::DisplayRole, item.data(Qt::DisplayRole));
    QCOMPARE(dataChangedSpy.count(), 0);
    QCOMPARE(itemChangedSpy.count(), 0);

    item.setFlags(Qt::ItemIsEnabled);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(itemChangedSpy.count(), 1);
    args = dataChangedSpy.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), index);
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(1)), index);
    args = itemChangedSpy.takeFirst();
    QCOMPARE(qvariant_cast<QStandardItem*>(args.at(0)), &item);

    item.setFlags(item.flags());
    QCOMPARE(dataChangedSpy.count(), 0);
    QCOMPARE(itemChangedSpy.count(), 0);
}

void tst_QStandardItemModel::takeHeaderItem()
{
    QStandardItemModel model;
    // set header items
    QStandardItem *hheader = new QStandardItem();
    model.setHorizontalHeaderItem(0, hheader);
    QStandardItem *vheader = new QStandardItem();
    model.setVerticalHeaderItem(0, vheader);
    // take header items
    QCOMPARE(model.takeHorizontalHeaderItem(0), hheader);
    QCOMPARE(model.takeVerticalHeaderItem(0), vheader);
    QCOMPARE(model.takeHorizontalHeaderItem(0), static_cast<QStandardItem*>(0));
    QCOMPARE(model.takeVerticalHeaderItem(0), static_cast<QStandardItem*>(0));
    delete hheader;
    delete vheader;
}

void tst_QStandardItemModel::useCase1()
{
    const int rows = 5;
    const int columns = 8;
    QStandardItemModel model(rows, columns);
    for (int i = 0; i < model.rowCount(); ++i) {
        for (int j = 0; j < model.columnCount(); ++j) {
            QCOMPARE(model.item(i, j), static_cast<QStandardItem*>(0));

            QStandardItem *item = new QStandardItem();
            model.setItem(i, j, item);
            QCOMPARE(item->row(), i);
            QCOMPARE(item->column(), j);

            QModelIndex index = model.indexFromItem(item);
            QCOMPARE(index, model.index(i, j, QModelIndex()));
            QStandardItem *sameItem = model.itemFromIndex(index);
            QCOMPARE(sameItem, item);
        }
    }
}

static void createChildren(QStandardItemModel *model, QStandardItem *parent, int level)
{
    if (level > 4)
        return;
    for (int i = 0; i < 4; ++i) {
        QCOMPARE(parent->rowCount(), i);
        parent->appendRow(QList<QStandardItem*>());
        for (int j = 0; j < parent->columnCount(); ++j) {
            QStandardItem *item = new QStandardItem();
            parent->setChild(i, j, item);
            QCOMPARE(item->row(), i);
            QCOMPARE(item->column(), j);

            QModelIndex parentIndex = model->indexFromItem(parent);
            QModelIndex index = model->indexFromItem(item);
            QCOMPARE(index, model->index(i, j, parentIndex));
            QStandardItem *theItem = model->itemFromIndex(index);
            QCOMPARE(theItem, item);
            QStandardItem *theParent = model->itemFromIndex(parentIndex);
            QCOMPARE(theParent, parent);
        }

        {
            QStandardItem *item = parent->child(i);
            item->setColumnCount(parent->columnCount());
            createChildren(model, item, level + 1);
        }
    }
}

void tst_QStandardItemModel::useCase2()
{
    QStandardItemModel model;
    model.setColumnCount(2);
    createChildren(&model, model.topLevelParent(), 0);
}

#endif // QT_VERSION >= 0x040200

QTEST_MAIN(tst_QStandardItemModel)
#include "tst_qstandarditemmodel.moc"
