/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtGui/QtGui>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qitemselectionmodel.h gui/itemviews/qitemselectionmodel.cpp

class tst_QItemSelectionModel : public QObject
{
    Q_OBJECT

public:
    tst_QItemSelectionModel();
    virtual ~tst_QItemSelectionModel();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
private slots:
    void clear_data();
    void clear();
    void clearAndSelect();
    void select_data();
    void select();
    void persistentselections_data();
    void persistentselections();
    void resetModel();
    void removeRows_data();
    void removeRows();
    void removeColumns_data();
    void removeColumns();
#if QT_VERSION >= 0x040200
    void modelLayoutChanged_data();
    void modelLayoutChanged();
#endif

private:
    QAbstractItemModel *model;
    QItemSelectionModel *selection;
};

QDataStream &operator<<(QDataStream &, const QModelIndex &);
QDataStream &operator>>(QDataStream &, QModelIndex &);
QDataStream &operator<<(QDataStream &, const QModelIndexList &);
QDataStream &operator>>(QDataStream &, QModelIndexList &);

typedef QList<int> IntList;
typedef QPair<int, int> IntPair;
typedef QList<IntPair> PairList;


Q_DECLARE_METATYPE(PairList)
Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QModelIndexList)
Q_DECLARE_METATYPE(IntList)
Q_DECLARE_METATYPE(QItemSelection)

class QStreamHelper: public QAbstractItemModel
{
public:
    QStreamHelper() {}
    static QModelIndex create(int row = -1, int column = -1, void *data = 0)
    {
        QStreamHelper helper;
        return helper.QAbstractItemModel::createIndex(row, column, data);
    }

    QModelIndex index(int, int, const QModelIndex&) const
        { return QModelIndex(); }
    QModelIndex parent(const QModelIndex&) const
        { return QModelIndex(); }
    int rowCount(const QModelIndex & = QModelIndex()) const
        { return 0; }
    int columnCount(const QModelIndex & = QModelIndex()) const
        { return 0; }
    QVariant data(const QModelIndex &, int = Qt::DisplayRole) const
        { return QVariant(); }
    bool hasChildren(const QModelIndex &) const
        { return false; }
};

QDataStream &operator<<(QDataStream &s, const QModelIndex &input)
{
    s << input.row()
      << input.column()
      << reinterpret_cast<qlonglong>(input.internalPointer());
    return s;
}

QDataStream &operator>>(QDataStream &s, QModelIndex &output)
{
    int r, c;
    qlonglong ptr;
    s >> r;
    s >> c;
    s >> ptr;
    output = QStreamHelper::create(r, c, reinterpret_cast<void *>(ptr));
    return s;
}

QDataStream &operator<<(QDataStream &s, const QModelIndexList &input)
{
    s << input.count();
    for (int i=0; i<input.count(); ++i)
        s << input.at(i);
    return s;
}

QDataStream &operator>>(QDataStream &s, QModelIndexList &output)
{
    QModelIndex tmpIndex;
    int count;
    s >> count;
    for (int i=0; i<count; ++i) {
        s >> tmpIndex;
        output << tmpIndex;
    }
    return s;
}

tst_QItemSelectionModel::tst_QItemSelectionModel() : model(0), selection(0)
{
}

tst_QItemSelectionModel::~tst_QItemSelectionModel()
{
}

/*
  This test usually uses a model with a 5x5 table
  -------------------------------------------
  |  0,0  |  0,1    |  0,2  |  0,3    |  0,4  |
  -------------------------------------------
  |  1,0  |  1,1    |  1,2  |  1,3    |  1,4  |
  -------------------------------------------
  |  2,0  |  2,1    |  2,2  |  2,3    |  2,4  |
  -------------------------------------------
  |  3,0  |  3,1    |  3,2  |  3,3    |  3,4  |
  -------------------------------------------
  |  4,0  |  4,1    |  4,2  |  4,3    |  4,4  |
  -------------------------------------------

  ...that for each row has a children in a new 5x5 table ad infinitum.

*/
void tst_QItemSelectionModel::initTestCase()
{
    qRegisterMetaType<QItemSelection>("QItemSelection");

    model = new QStandardItemModel(5, 5);
    QModelIndex parent = model->index(0, 0, QModelIndex());
    model->insertRows(0, 5, parent);
    model->insertColumns(0, 5, parent);
    selection  = new QItemSelectionModel(model);
}

void tst_QItemSelectionModel::cleanupTestCase()
{
    delete selection;
    delete model;
}

void tst_QItemSelectionModel::init()
{
    selection->clear();
    while (model->rowCount(QModelIndex()) > 5)
        model->removeRow(0, QModelIndex());
    while (model->rowCount(QModelIndex()) < 5)
        model->insertRow(0, QModelIndex());
}

void tst_QItemSelectionModel::clear_data()
{
    QTest::addColumn<QModelIndexList>("indexList");
    QTest::addColumn<IntList>("commandList");
    {
        QModelIndexList index;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Rows);
        index << model->index(1, 0, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Rows);
        QTest::newRow("(0, 0) and (1, 0): Select|Rows")
            << index
            << command;
    }
    {
        QModelIndexList index;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Columns);
        index << model->index(0, 1, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Columns);
        QTest::newRow("(0, 0) and (1, 0): Select|Columns")
            << index
            << command;
    }
    {
        QModelIndexList index;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(1, 1, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::SelectCurrent;
        QTest::newRow("(0, 0), (1, 1) and (2, 2): Select, Select, SelectCurrent")
            << index
            << command;
    }
    {
        QModelIndexList index;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(1, 1, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(1, 1, QModelIndex());
        command << QItemSelectionModel::Toggle;
        QTest::newRow("(0, 0), (1, 1) and (1, 1): Select, Select, Toggle")
            << index
            << command;
    }
    {
        QModelIndexList index;
        IntList command;
        index << model->index(0, 0, model->index(0, 0, QModelIndex()));
        command << (QItemSelectionModel::Select | QItemSelectionModel::Rows);
        QTest::newRow("child (0, 0) of (0, 0): Select|Rows")
            << index
            << command;
    }
}

void tst_QItemSelectionModel::clear()
{
    QFETCH(QModelIndexList, indexList);
    QFETCH(IntList, commandList);

    // do selections
    for (int i=0; i<indexList.count(); ++i) {
        selection->select(indexList.at(i), (QItemSelectionModel::SelectionFlags)commandList.at(i));
    }
    // test that we have selected items
    QVERIFY(!selection->selectedIndexes().isEmpty());
    selection->clear();
    // test that they were all cleared
    QVERIFY(selection->selectedIndexes().isEmpty());
}

void tst_QItemSelectionModel::clearAndSelect()
{
    // populate selectionmodel
    selection->select(model->index(1, 1, QModelIndex()), QItemSelectionModel::Select);
    QCOMPARE(selection->selectedIndexes().count(), 1);

    // ClearAndSelect with empty selection
    QItemSelection emptySelection;
    selection->select(emptySelection, QItemSelectionModel::ClearAndSelect);

    // verify the selectionmodel is empty
    QVERIFY(selection->selectedIndexes().isEmpty());
}

void tst_QItemSelectionModel::select_data()
{
    QTest::addColumn<QModelIndexList>("indexList");
    QTest::addColumn<bool>("useRanges");
    QTest::addColumn<IntList>("commandList");
    QTest::addColumn<QModelIndexList>("expectedList");

    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        expected << model->index(0, 0, QModelIndex());
        QTest::newRow("(0, 0): Select")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, model->index(0, 0, QModelIndex()));
        command << QItemSelectionModel::Select;
        expected << model->index(0, 0, model->index(0, 0, QModelIndex()));
        QTest::newRow("child (0, 0) of (0, 0): Select")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Deselect;
        QTest::newRow("(0, 0): Deselect")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Toggle;
        expected << model->index(0, 0, QModelIndex());
        QTest::newRow("(0, 0): Toggle")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Toggle;
        QTest::newRow("(0, 0) and (0, 0): Select and Toggle")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Deselect;
        QTest::newRow("(0, 0) and (0, 0): Select and Deselect")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(0, 0, model->index(0, 0, QModelIndex()));
        command << QItemSelectionModel::ClearAndSelect;
        expected << model->index(0, 0, model->index(0, 0, QModelIndex()));
        QTest::newRow("(0, 0) and child (0, 0) of (0, 0): Select and ClearAndSelect")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(4, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(0, 1, QModelIndex());
        index << model->index(4, 1, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(0, 0, QModelIndex());
        index << model->index(4, 1, QModelIndex());
        command << QItemSelectionModel::Deselect;
        QTest::newRow("(0, 0 to 4, 0) and (0, 1 to 4, 1) and (0, 0 to 4, 1): Select and Select and Deselect")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(4, 4, QModelIndex());
        command << QItemSelectionModel::Select;
        expected << model->index(0, 0, QModelIndex()) << model->index(4, 4, QModelIndex());
        QTest::newRow("(0, 0) and (4, 4): Select")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(4, 4, QModelIndex());
        command << QItemSelectionModel::ClearAndSelect;
        expected << model->index(4, 4, QModelIndex());
        QTest::newRow("(0, 0) and (4, 4): Select and ClearAndSelect")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Rows);
        index << model->index(4, 4, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Rows);
        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(0, 3, QModelIndex())
                 << model->index(0, 4, QModelIndex())
                 << model->index(4, 0, QModelIndex())
                 << model->index(4, 1, QModelIndex())
                 << model->index(4, 2, QModelIndex())
                 << model->index(4, 3, QModelIndex())
                 << model->index(4, 4, QModelIndex());
        QTest::newRow("(0, 0) and (4, 4): Select|Rows")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, model->index(0, 0, QModelIndex()));
        command << (QItemSelectionModel::Select | QItemSelectionModel::Rows);
        index << model->index(4, 4, model->index(0, 0, QModelIndex()));
        command << (QItemSelectionModel::Select | QItemSelectionModel::Rows);
        QModelIndex parent = model->index(0, 0, QModelIndex());
        expected << model->index(0, 0, parent)
                 << model->index(0, 1, parent)
                 << model->index(0, 2, parent)
                 << model->index(0, 3, parent)
                 << model->index(0, 4, parent)
                 << model->index(4, 0, parent)
                 << model->index(4, 1, parent)
                 << model->index(4, 2, parent)
                 << model->index(4, 3, parent)
                 << model->index(4, 4, parent);
        QTest::newRow("child (0, 0) and (4, 4) of (0, 0): Select|Rows")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Columns);
        index << model->index(4, 4, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Columns);
        expected << model->index(0, 0, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(3, 0, QModelIndex())
                 << model->index(4, 0, QModelIndex())
                 << model->index(0, 4, QModelIndex())
                 << model->index(1, 4, QModelIndex())
                 << model->index(2, 4, QModelIndex())
                 << model->index(3, 4, QModelIndex())
                 << model->index(4, 4, QModelIndex());
        QTest::newRow("(0, 0) and (4, 4): Select|Columns")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, model->index(0, 0, QModelIndex()));
        command << (QItemSelectionModel::Select | QItemSelectionModel::Columns);
        index << model->index(4, 4, model->index(0, 0, QModelIndex()));
        command << (QItemSelectionModel::Select | QItemSelectionModel::Columns);
        expected << model->index(0, 0, model->index(0, 0, QModelIndex()))
                 << model->index(1, 0, model->index(0, 0, QModelIndex()))
                 << model->index(2, 0, model->index(0, 0, QModelIndex()))
                 << model->index(3, 0, model->index(0, 0, QModelIndex()))
                 << model->index(4, 0, model->index(0, 0, QModelIndex()))
                 << model->index(0, 4, model->index(0, 0, QModelIndex()))
                 << model->index(1, 4, model->index(0, 0, QModelIndex()))
                 << model->index(2, 4, model->index(0, 0, QModelIndex()))
                 << model->index(3, 4, model->index(0, 0, QModelIndex()))
                 << model->index(4, 4, model->index(0, 0, QModelIndex()));
        QTest::newRow("child (0, 0) and (4, 4) of (0, 0): Select|Columns")
            << index
            << false
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(4, 0, QModelIndex());
        command << QItemSelectionModel::Select;
        expected << model->index(0, 0, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(3, 0, QModelIndex())
                 << model->index(4, 0, QModelIndex());
        QTest::newRow("(0, 0 to 4, 0): Select")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(0, 0, model->index(0, 0, QModelIndex()));
        command << QItemSelectionModel::Select;
        QTest::newRow("(0, 0 to child 0, 0): Select")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, model->index(0, 0, QModelIndex()));
        index << model->index(0, 0, model->index(1, 0, QModelIndex()));
        command << QItemSelectionModel::Select;
        QTest::newRow("child (0, 0) of (0, 0) to child (0, 0) of (1, 0): Select")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(4, 4, QModelIndex());
        command << QItemSelectionModel::Select;
        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(0, 3, QModelIndex())
                 << model->index(0, 4, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(1, 3, QModelIndex())
                 << model->index(1, 4, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex())
                 << model->index(2, 3, QModelIndex())
                 << model->index(2, 4, QModelIndex())
                 << model->index(3, 0, QModelIndex())
                 << model->index(3, 1, QModelIndex())
                 << model->index(3, 2, QModelIndex())
                 << model->index(3, 3, QModelIndex())
                 << model->index(3, 4, QModelIndex())
                 << model->index(4, 0, QModelIndex())
                 << model->index(4, 1, QModelIndex())
                 << model->index(4, 2, QModelIndex())
                 << model->index(4, 3, QModelIndex())
                 << model->index(4, 4, QModelIndex());
        QTest::newRow("(0, 0 to 4, 4): Select")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(4, 0, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Rows);
        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(0, 3, QModelIndex())
                 << model->index(0, 4, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(1, 3, QModelIndex())
                 << model->index(1, 4, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex())
                 << model->index(2, 3, QModelIndex())
                 << model->index(2, 4, QModelIndex())
                 << model->index(3, 0, QModelIndex())
                 << model->index(3, 1, QModelIndex())
                 << model->index(3, 2, QModelIndex())
                 << model->index(3, 3, QModelIndex())
                 << model->index(3, 4, QModelIndex())
                 << model->index(4, 0, QModelIndex())
                 << model->index(4, 1, QModelIndex())
                 << model->index(4, 2, QModelIndex())
                 << model->index(4, 3, QModelIndex())
                 << model->index(4, 4, QModelIndex());
        QTest::newRow("(0, 0 to 4, 0): Select|Rows")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(0, 4, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Columns);
        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(0, 3, QModelIndex())
                 << model->index(0, 4, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(1, 3, QModelIndex())
                 << model->index(1, 4, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex())
                 << model->index(2, 3, QModelIndex())
                 << model->index(2, 4, QModelIndex())
                 << model->index(3, 0, QModelIndex())
                 << model->index(3, 1, QModelIndex())
                 << model->index(3, 2, QModelIndex())
                 << model->index(3, 3, QModelIndex())
                 << model->index(3, 4, QModelIndex())
                 << model->index(4, 0, QModelIndex())
                 << model->index(4, 1, QModelIndex())
                 << model->index(4, 2, QModelIndex())
                 << model->index(4, 3, QModelIndex())
                 << model->index(4, 4, QModelIndex());
        QTest::newRow("(0, 0 to 0, 4): Select|Columns")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(4, 4, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Rows);
        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(0, 3, QModelIndex())
                 << model->index(0, 4, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(1, 3, QModelIndex())
                 << model->index(1, 4, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex())
                 << model->index(2, 3, QModelIndex())
                 << model->index(2, 4, QModelIndex())
                 << model->index(3, 0, QModelIndex())
                 << model->index(3, 1, QModelIndex())
                 << model->index(3, 2, QModelIndex())
                 << model->index(3, 3, QModelIndex())
                 << model->index(3, 4, QModelIndex())
                 << model->index(4, 0, QModelIndex())
                 << model->index(4, 1, QModelIndex())
                 << model->index(4, 2, QModelIndex())
                 << model->index(4, 3, QModelIndex())
                 << model->index(4, 4, QModelIndex());
        QTest::newRow("(0, 0 to 4, 4): Select|Rows")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(4, 4, QModelIndex());
        command << (QItemSelectionModel::Select | QItemSelectionModel::Columns);
        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(0, 3, QModelIndex())
                 << model->index(0, 4, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(1, 3, QModelIndex())
                 << model->index(1, 4, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex())
                 << model->index(2, 3, QModelIndex())
                 << model->index(2, 4, QModelIndex())
                 << model->index(3, 0, QModelIndex())
                 << model->index(3, 1, QModelIndex())
                 << model->index(3, 2, QModelIndex())
                 << model->index(3, 3, QModelIndex())
                 << model->index(3, 4, QModelIndex())
                 << model->index(4, 0, QModelIndex())
                 << model->index(4, 1, QModelIndex())
                 << model->index(4, 2, QModelIndex())
                 << model->index(4, 3, QModelIndex())
                 << model->index(4, 4, QModelIndex());
        QTest::newRow("(0, 0 to 4, 4): Select|Columns")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 2, QModelIndex());
        index << model->index(4, 2, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(2, 0, QModelIndex());
        index << model->index(2, 4, QModelIndex());
        command << QItemSelectionModel::Select;
        expected << model->index(0, 2, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(2, 2, QModelIndex())
                 << model->index(3, 2, QModelIndex())
                 << model->index(4, 2, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 3, QModelIndex())
                 << model->index(2, 4, QModelIndex());
        QTest::newRow("(0, 2 to 4, 2) and (2, 0 to 2, 4): Select")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 2, QModelIndex());
        index << model->index(4, 2, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(2, 0, QModelIndex());
        index << model->index(2, 4, QModelIndex());
        command << QItemSelectionModel::SelectCurrent;
        expected << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex())
                 << model->index(2, 3, QModelIndex())
                 << model->index(2, 4, QModelIndex());
        QTest::newRow("(0, 2 to 4, 2) and (2, 0 to 2, 4): Select and SelectCurrent")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 2, QModelIndex());
        index << model->index(4, 2, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(2, 0, QModelIndex());
        index << model->index(2, 4, QModelIndex());
        command << QItemSelectionModel::Toggle;
        expected << model->index(0, 2, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(3, 2, QModelIndex())
                 << model->index(4, 2, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 3, QModelIndex())
                 << model->index(2, 4, QModelIndex());
        QTest::newRow("(0, 2 to 4, 2) and (2, 0 to 2, 4): Select and Toggle")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 2, QModelIndex());
        index << model->index(4, 2, QModelIndex());
        command << QItemSelectionModel::Select;
        index << model->index(2, 0, QModelIndex());
        index << model->index(2, 4, QModelIndex());
        command << QItemSelectionModel::Deselect;
        expected << model->index(0, 2, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(3, 2, QModelIndex())
                 << model->index(4, 2, QModelIndex());
        QTest::newRow("(0, 2 to 4, 2) and (2, 0 to 2, 4): Select and Deselect")
            << index
            << true
            << command
            << expected;
    }

    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Select;

        index << model->index(0, 0, QModelIndex());
        index << model->index(0, 0, QModelIndex());
        command << QItemSelectionModel::Toggle;

        expected << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex());

        QTest::newRow("(0, 0 to 2, 2) and (0, 0 to 0, 0): Select and Toggle at selection boundary")
            << index
            << true
            << command
            << expected;
    }

    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Select;

        index << model->index(0, 1, QModelIndex());
        index << model->index(0, 1, QModelIndex());
        command << QItemSelectionModel::Toggle;

        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex());

        QTest::newRow("(0, 0 to 2, 2) and (0, 1 to 0, 1): Select and Toggle at selection boundary")
            << index
            << true
            << command
            << expected;
    }

    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Select;

        index << model->index(0, 2, QModelIndex());
        index << model->index(0, 2, QModelIndex());
        command << QItemSelectionModel::Toggle;

        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex());

        QTest::newRow("(0, 0 to 2, 2) and (0, 2 to 0, 2): Select and Toggle at selection boundary")
            << index
            << true
            << command
            << expected;
    }

    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Select;

        index << model->index(1, 0, QModelIndex());
        index << model->index(1, 0, QModelIndex());
        command << QItemSelectionModel::Toggle;

        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex());

        QTest::newRow("(0, 0 to 2, 2) and (1, 0 to 1, 0): Select and Toggle at selection boundary")
            << index
            << true
            << command
            << expected;
    }

    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Select;

        index << model->index(1, 1, QModelIndex());
        index << model->index(1, 1, QModelIndex());
        command << QItemSelectionModel::Toggle;

        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex());

        QTest::newRow("(0, 0 to 2, 2) and (1, 1 to 1, 1): Select and Toggle at selection boundary")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Select;

        index << model->index(1, 2, QModelIndex());
        index << model->index(1, 2, QModelIndex());
        command << QItemSelectionModel::Toggle;

        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex());

        QTest::newRow("(0, 0 to 2, 2) and (1, 2 to 1, 2): Select and Toggle at selection boundary")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Select;

        index << model->index(2, 0, QModelIndex());
        index << model->index(2, 0, QModelIndex());
        command << QItemSelectionModel::Toggle;

        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(2, 1, QModelIndex())
                 << model->index(2, 2, QModelIndex());

        QTest::newRow("(0, 0 to 2, 2) and (2, 0 to 2, 0): Select and Toggle at selection boundary")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Select;

        index << model->index(2, 1, QModelIndex());
        index << model->index(2, 1, QModelIndex());
        command << QItemSelectionModel::Toggle;

        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 2, QModelIndex());

        QTest::newRow("(0, 0 to 2, 2) and (2, 1 to 2, 1): Select and Toggle at selection boundary")
            << index
            << true
            << command
            << expected;
    }
    {
        QModelIndexList index;
        QModelIndexList expected;
        IntList command;
        index << model->index(0, 0, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Select;

        index << model->index(2, 2, QModelIndex());
        index << model->index(2, 2, QModelIndex());
        command << QItemSelectionModel::Toggle;

        expected << model->index(0, 0, QModelIndex())
                 << model->index(0, 1, QModelIndex())
                 << model->index(0, 2, QModelIndex())
                 << model->index(1, 0, QModelIndex())
                 << model->index(1, 1, QModelIndex())
                 << model->index(1, 2, QModelIndex())
                 << model->index(2, 0, QModelIndex())
                 << model->index(2, 1, QModelIndex());

        QTest::newRow("(0, 0 to 2, 2) and (2, 2 to 2, 2): Select and Toggle at selection boundary")
            << index
            << true
            << command
            << expected;
    }

}

void tst_QItemSelectionModel::select()
{
    QFETCH(QModelIndexList, indexList);
    QFETCH(bool, useRanges);
    QFETCH(IntList, commandList);
    QFETCH(QModelIndexList, expectedList);

    int lastCommand = 0;
    // do selections
    for (int i = 0; i<commandList.count(); ++i) {
        if (useRanges) {
            selection->select(QItemSelection(indexList.at(2*i), indexList.at(2*i+1)),
                              (QItemSelectionModel::SelectionFlags)commandList.at(i));
        } else {
            selection->select(indexList.at(i),
                              (QItemSelectionModel::SelectionFlags)commandList.at(i));
        }
        lastCommand = commandList.at(i);
    }


    QModelIndexList selectedList = selection->selectedIndexes();
    // debug output
//     for (int i=0; i<selectedList.count(); ++i)
//         qDebug(QString("selected (%1, %2)")
//                .arg(selectedList.at(i).row())
//                .arg(selectedList.at(i).column()));

    // test that the number of indices are as expected
    QVERIFY2(selectedList.count() == expectedList.count(),
            QString("expected indices: %1 actual indices: %2")
            .arg(expectedList.count())
            .arg(selectedList.count()).toLatin1());

    // test existence of each index
    for (int i=0; i<expectedList.count(); ++i)
        QVERIFY2(selectedList.contains(expectedList.at(i)),
                QString("expected index(%1, %2) not found in selectedIndexes()")
                .arg(expectedList.at(i).row())
                .arg(expectedList.at(i).column()).toLatin1());

    // test that isSelected agrees
    for (int i=0; i<selectedList.count(); ++i)
        QVERIFY2(selection->isSelected(selectedList.at(i)), QString("isSelected(index: %1, %2) does not match selectedIndexes()")
               .arg(selectedList.at(i).row())
               .arg(selectedList.at(i).column()).toLatin1());

    //for now we assume Rows/Columns flag is the same for all commands, therefore we just check lastCommand
    // test that isRowSelected agrees
    if (lastCommand & QItemSelectionModel::Rows) {
        for (int i=0; i<selectedList.count(); ++i)
            QVERIFY2(selection->isRowSelected(selectedList.at(i).row(),
                                             model->parent(selectedList.at(i))),
                    QString("isRowSelected(row: %1) does not match selectedIndexes()")
                    .arg(selectedList.at(i).row()).toLatin1());
    }

    // test that isColumnSelected agrees
    if (lastCommand & QItemSelectionModel::Columns) {
        for (int i=0; i<selectedList.count(); ++i)
            QVERIFY2(selection->isColumnSelected(selectedList.at(i).column(),
                                                model->parent(selectedList.at(i))),
                    QString("isColumnSelected(column: %1) does not match selectedIndexes()")
                    .arg(selectedList.at(i).column()).toLatin1());
    }
}

void tst_QItemSelectionModel::persistentselections_data()
{
    QTest::addColumn<PairList>("indexList");
    QTest::addColumn<IntList>("commandList");
    QTest::addColumn<IntList>("insertRows"); // start, count
    QTest::addColumn<IntList>("insertColumns"); // start, count
    QTest::addColumn<IntList>("deleteRows"); // start, count
    QTest::addColumn<IntList>("deleteColumns"); // start, count
    QTest::addColumn<PairList>("expectedList");

    PairList index, expected;
    IntList command, insertRows, insertColumns, deleteRows, deleteColumns;


    index.clear(); expected.clear(); command.clear();
    insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
    index << IntPair(0, 0);
    command << QItemSelectionModel::ClearAndSelect;
    deleteRows << 4 << 1;
    expected << IntPair(0, 0);
    QTest::newRow("ClearAndSelect (0, 0). Delete last row.")
        << index << command
        << insertRows << insertColumns << deleteRows << deleteColumns
        << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(0, 0);
     command << QItemSelectionModel::ClearAndSelect;
     deleteRows << 0 << 1;
     QTest::newRow("ClearAndSelect (0, 0). Delete first row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(1, 0);
     command << QItemSelectionModel::ClearAndSelect;
     deleteRows << 0 << 1;
     expected << IntPair(0, 0);
     QTest::newRow("ClearAndSelect (1, 0). Delete first row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(0, 0);
     command << QItemSelectionModel::ClearAndSelect;
     insertRows << 5 << 1;
     expected << IntPair(0, 0);
     QTest::newRow("ClearAndSelect (0, 0). Append row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(0, 0);
     command << QItemSelectionModel::ClearAndSelect;
     insertRows << 0 << 1;
     expected << IntPair(1, 0);
     QTest::newRow("ClearAndSelect (0, 0). Insert before first row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(0, 0)
           << IntPair(4, 0);
     command << QItemSelectionModel::ClearAndSelect;
     insertRows << 5 << 1;
     expected << IntPair(0, 0)
              << IntPair(1, 0)
              << IntPair(2, 0)
              << IntPair(3, 0)
              << IntPair(4, 0);
     QTest::newRow("ClearAndSelect (0, 0) to (4, 0). Append row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(0, 0)
           << IntPair(4, 0);
     command << QItemSelectionModel::ClearAndSelect;
     insertRows << 0  << 1;
     expected << IntPair(1, 0)
              << IntPair(2, 0)
              << IntPair(3, 0)
              << IntPair(4, 0)
              << IntPair(5, 0);
     QTest::newRow("ClearAndSelect (0, 0) to (4, 0). Insert before first row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(0, 0)
           << IntPair(4, 0);
     command << QItemSelectionModel::ClearAndSelect;
     deleteRows << 0  << 1;
     expected << IntPair(0, 0)
              << IntPair(1, 0)
              << IntPair(2, 0)
              << IntPair(3, 0);
     QTest::newRow("ClearAndSelect (0, 0) to (4, 0). Delete first row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(0, 0)
           << IntPair(4, 0);
     command << QItemSelectionModel::ClearAndSelect;
     deleteRows << 4  << 1;
     expected << IntPair(0, 0)
              << IntPair(1, 0)
              << IntPair(2, 0)
              << IntPair(3, 0);
     QTest::newRow("ClearAndSelect (0, 0) to (4, 0). Delete last row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(0, 0)
           << IntPair(4, 0);
     command << QItemSelectionModel::ClearAndSelect;
     deleteRows << 1  << 3;
     expected << IntPair(0, 0)
              << IntPair(1, 0);
     QTest::newRow("ClearAndSelect (0, 0) to (4, 0). Deleting all but first and last row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;

     index.clear(); expected.clear(); command.clear();
     insertRows.clear(); insertColumns.clear(); deleteRows.clear(); deleteColumns.clear();
     index << IntPair(0, 0)
           << IntPair(4, 0);
     command << QItemSelectionModel::ClearAndSelect;
     insertRows << 1 << 1;
     expected << IntPair(0, 0)
         // the inserted row should not be selected
              << IntPair(2, 0)
              << IntPair(3, 0)
              << IntPair(4, 0)
              << IntPair(5, 0);
     QTest::newRow("ClearAndSelect (0, 0) to (4, 0). Insert after first row.")
         << index << command
         << insertRows << insertColumns << deleteRows << deleteColumns
         << expected;
}

void tst_QItemSelectionModel::persistentselections()
{
    QFETCH(PairList, indexList);
    QFETCH(IntList, commandList);
    QFETCH(IntList, insertRows);
    QFETCH(IntList, insertColumns);
    QFETCH(IntList, deleteRows);
    QFETCH(IntList, deleteColumns);
    QFETCH(PairList, expectedList);

    // make sure the model is sane (5x5)
    QCOMPARE(model->rowCount(QModelIndex()), 5);
    QCOMPARE(model->columnCount(QModelIndex()), 5);

    // do selections
    for (int i=0; i<commandList.count(); ++i) {
        if (indexList.count() == commandList.count()) {
            QModelIndex index = model->index(indexList.at(i).first,
                                             indexList.at(i).second,
                                             QModelIndex());
            selection->select(index, (QItemSelectionModel::SelectionFlags)commandList.at(i));
        } else {
            QModelIndex tl = model->index(indexList.at(2*i).first,
                                          indexList.at(2*i).second,
                                          QModelIndex());
            QModelIndex br = model->index(indexList.at(2*i+1).first,
                                          indexList.at(2*i+1).second,
                                          QModelIndex());
            selection->select(QItemSelection(tl, br),
                              (QItemSelectionModel::SelectionFlags)commandList.at(i));
        }
    }
    // test that we have selected items
    QVERIFY(!selection->selectedIndexes().isEmpty());

    // insert/delete row and/or columns
    if (insertRows.count() > 1)
        model->insertRows(insertRows.at(0), insertRows.at(1), QModelIndex());
    if (insertColumns.count() > 1)
        model->insertColumns(insertColumns.at(0), insertColumns.at(1), QModelIndex());
    if (deleteRows.count() > 1)
        model->removeRows(deleteRows.at(0), deleteRows.at(1), QModelIndex());
    if (deleteColumns.count() > 1)
        model->removeColumns(deleteColumns.at(0), deleteColumns.at(1), QModelIndex());

    // check that the selected items are the correct number and indexes
    QModelIndexList selectedList = selection->selectedIndexes();
    QCOMPARE(selectedList.count(), expectedList.count());
    foreach(IntPair pair, expectedList) {
        QModelIndex index = model->index(pair.first, pair.second, QModelIndex());
        QVERIFY(selectedList.contains(index));
    }
}

// "make reset public"-model
class MyStandardItemModel: public QStandardItemModel
{
    Q_OBJECT
public:
    inline MyStandardItemModel(int i1, int i2): QStandardItemModel(i1, i2) {}
    inline void reset() { QStandardItemModel::reset(); }
};

void tst_QItemSelectionModel::resetModel()
{
    MyStandardItemModel model(20, 20);
    QTreeView view;
    view.setModel(&model);

    QSignalSpy spy(view.selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)));

    view.selectionModel()->select(QItemSelection(model.index(0, 0), model.index(5, 5)), QItemSelectionModel::Select);

    QCOMPARE(spy.count(), 1);

    model.reset();

    QVERIFY(view.selectionModel()->selection().isEmpty());

    view.selectionModel()->select(QItemSelection(model.index(0, 0), model.index(5, 5)), QItemSelectionModel::Select);

    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).count(), 2);
    // make sure we don't get an "old selection"
    QCOMPARE(spy.at(1).at(1).userType(), qMetaTypeId<QItemSelection>());
    QVERIFY(qvariant_cast<QItemSelection>(spy.at(1).at(1)).isEmpty());
}

void tst_QItemSelectionModel::removeRows_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");

    QTest::addColumn<int>("selectTop");
    QTest::addColumn<int>("selectLeft");
    QTest::addColumn<int>("selectBottom");
    QTest::addColumn<int>("selectRight");

    QTest::addColumn<int>("removeTop");
    QTest::addColumn<int>("removeBottom");

    QTest::addColumn<int>("expectedTop");
    QTest::addColumn<int>("expectedLeft");
    QTest::addColumn<int>("expectedBottom");
    QTest::addColumn<int>("expectedRight");

    QTest::newRow("4x4 <0,1><1,1>")
        << 4 << 4
        << 0 << 1 << 1 << 1
        << 0 << 0
        << 0 << 1 << 0 << 1;
}

void tst_QItemSelectionModel::removeRows()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, selectTop);
    QFETCH(int, selectLeft);
    QFETCH(int, selectBottom);
    QFETCH(int, selectRight);
    QFETCH(int, removeTop);
    QFETCH(int, removeBottom);
    QFETCH(int, expectedTop);
    QFETCH(int, expectedLeft);
    QFETCH(int, expectedBottom);
    QFETCH(int, expectedRight);

    MyStandardItemModel model(rowCount, columnCount);
    QItemSelectionModel selections(&model);
    QSignalSpy spy(&selections, SIGNAL(selectionChanged(QItemSelection,QItemSelection)));

    QModelIndex tl = model.index(selectTop, selectLeft);
    QModelIndex br = model.index(selectBottom, selectRight);
    selections.select(QItemSelection(tl, br), QItemSelectionModel::ClearAndSelect);

    QCOMPARE(spy.count(), 1);
    QVERIFY(selections.isSelected(tl));
    QVERIFY(selections.isSelected(br));

    model.removeRows(removeTop, removeBottom - removeTop + 1);

    QCOMPARE(spy.count(), 2);
    tl = model.index(expectedTop, expectedLeft);
    br = model.index(expectedBottom, expectedRight);
    QVERIFY(selections.isSelected(tl));
    QVERIFY(selections.isSelected(br));
}

void tst_QItemSelectionModel::removeColumns_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");

    QTest::addColumn<int>("selectTop");
    QTest::addColumn<int>("selectLeft");
    QTest::addColumn<int>("selectBottom");
    QTest::addColumn<int>("selectRight");

    QTest::addColumn<int>("removeLeft");
    QTest::addColumn<int>("removeRight");

    QTest::addColumn<int>("expectedTop");
    QTest::addColumn<int>("expectedLeft");
    QTest::addColumn<int>("expectedBottom");
    QTest::addColumn<int>("expectedRight");

    QTest::newRow("4x4 <0,1><1,1>")
        << 4 << 4
        << 1 << 0 << 1 << 1
        << 0 << 0
        << 1 << 0 << 1 << 0;
}

void tst_QItemSelectionModel::removeColumns()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, selectTop);
    QFETCH(int, selectLeft);
    QFETCH(int, selectBottom);
    QFETCH(int, selectRight);
    QFETCH(int, removeLeft);
    QFETCH(int, removeRight);
    QFETCH(int, expectedTop);
    QFETCH(int, expectedLeft);
    QFETCH(int, expectedBottom);
    QFETCH(int, expectedRight);

    MyStandardItemModel model(rowCount, columnCount);
    QItemSelectionModel selections(&model);
    QSignalSpy spy(&selections, SIGNAL(selectionChanged(QItemSelection,QItemSelection)));

    QModelIndex tl = model.index(selectTop, selectLeft);
    QModelIndex br = model.index(selectBottom, selectRight);
    selections.select(QItemSelection(tl, br), QItemSelectionModel::ClearAndSelect);

    QCOMPARE(spy.count(), 1);
    QVERIFY(selections.isSelected(tl));
    QVERIFY(selections.isSelected(br));

    model.removeColumns(removeLeft, removeRight - removeLeft + 1);

    QCOMPARE(spy.count(), 2);
    tl = model.index(expectedTop, expectedLeft);
    br = model.index(expectedBottom, expectedRight);
    QVERIFY(selections.isSelected(tl));
    QVERIFY(selections.isSelected(br));
}

#if QT_VERSION >= 0x040200
typedef QList<IntList> IntListList;
typedef QPair<IntPair, IntPair> IntPairPair;
typedef QList<IntPairPair> IntPairPairList;
Q_DECLARE_METATYPE(IntListList)
Q_DECLARE_METATYPE(IntPairPair)
Q_DECLARE_METATYPE(IntPairPairList)

void tst_QItemSelectionModel::modelLayoutChanged_data()
{
    QTest::addColumn<IntListList>("items");
    QTest::addColumn<IntPairPairList>("initialSelectedRanges");
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<int>("sortColumn");
    QTest::addColumn<IntPairPairList>("expectedSelectedRanges");

    QTest::newRow("everything selected, then row order reversed")
        << (IntListList()
            << (IntList() << 0 << 1 << 2 << 3)
            << (IntList() << 3 << 2 << 1 << 0))
        << (IntPairPairList()
            << IntPairPair(IntPair(0, 0), IntPair(3, 1)))
        << int(Qt::DescendingOrder)
        << 0
        << (IntPairPairList()
            << IntPairPair(IntPair(0, 0), IntPair(3, 1)));
    QTest::newRow("first two rows selected, then row order reversed")
        << (IntListList()
            << (IntList() << 0 << 1 << 2 << 3)
            << (IntList() << 3 << 2 << 1 << 0))
        << (IntPairPairList()
            << IntPairPair(IntPair(0, 0), IntPair(1, 1)))
        << int(Qt::DescendingOrder)
        << 0
        << (IntPairPairList()
            << IntPairPair(IntPair(2, 0), IntPair(3, 1)));
    QTest::newRow("middle two rows selected, then row order reversed")
        << (IntListList()
            << (IntList() << 0 << 1 << 2 << 3)
            << (IntList() << 3 << 2 << 1 << 0))
        << (IntPairPairList()
            << IntPairPair(IntPair(1, 0), IntPair(2, 1)))
        << int(Qt::DescendingOrder)
        << 0
        << (IntPairPairList()
            << IntPairPair(IntPair(1, 0), IntPair(2, 1)));
    QTest::newRow("two ranges")
        << (IntListList()
            << (IntList() << 2 << 0 << 3 << 1)
            << (IntList() << 2 << 0 << 3 << 1))
        << (IntPairPairList()
            << IntPairPair(IntPair(1, 0), IntPair(1, 1))
            << IntPairPair(IntPair(3, 0), IntPair(3, 1)))
        << int(Qt::AscendingOrder)
        << 0
        << (IntPairPairList()
            << IntPairPair(IntPair(0, 0), IntPair(0, 1))
            << IntPairPair(IntPair(1, 0), IntPair(1, 1)));
}

void tst_QItemSelectionModel::modelLayoutChanged()
{
    QFETCH(IntListList, items);
    QFETCH(IntPairPairList, initialSelectedRanges);
    QFETCH(int, sortOrder);
    QFETCH(int, sortColumn);
    QFETCH(IntPairPairList, expectedSelectedRanges);

    MyStandardItemModel model(items.at(0).count(), items.count());
    // initialize model data
    for (int i = 0; i < model.rowCount(); ++i) {
        for (int j = 0; j < model.columnCount(); ++j) {
            QModelIndex index = model.index(i, j);
            model.setData(index, items.at(j).at(i), Qt::DisplayRole);
        }
    }

    // select initial ranges
    QItemSelectionModel selectionModel(&model);
    foreach (IntPairPair range, initialSelectedRanges) {
        IntPair tl = range.first;
        IntPair br = range.second;
        QItemSelection selection(
            model.index(tl.first, tl.second),
            model.index(br.first, br.second));
        selectionModel.select(selection, QItemSelectionModel::Select);
    }

    // sort the model
    model.sort(sortColumn, Qt::SortOrder(sortOrder));

    // verify that selection is as expected
    QItemSelection selection = selectionModel.selection();
    QCOMPARE(selection.count(), expectedSelectedRanges.count());
    for (int i = 0; i < expectedSelectedRanges.count(); ++i) {
        IntPairPair expectedRange = expectedSelectedRanges.at(i);
        IntPair expectedTl = expectedRange.first;
        IntPair expectedBr = expectedRange.second;
        QItemSelectionRange actualRange = selection.at(i);
        QModelIndex actualTl = actualRange.topLeft();
        QModelIndex actualBr = actualRange.bottomRight();
        QCOMPARE(actualTl.row(), expectedTl.first);
        QCOMPARE(actualTl.column(), expectedTl.second);
        QCOMPARE(actualBr.row(), expectedBr.first);
        QCOMPARE(actualBr.column(), expectedBr.second);
    }
}
#endif

QTEST_MAIN(tst_QItemSelectionModel)
#include "tst_qitemselectionmodel.moc"
