/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtCore>
#include <QtGui>

#include <qdebug.h>

#if QT_VERSION >= 0x040100

//TESTED CLASS=
//TESTED_FILES=gui/itemviews/qmappingproxymodel.h gui/itemviews/qmappingproxymodel.cpp

#if QT_VERSION >= 0x040200

typedef QList<int> IntList;
typedef QPair<int, int> IntPair;
typedef QList<IntPair> IntPairList;

Q_DECLARE_METATYPE(IntList)
Q_DECLARE_METATYPE(IntPair)
Q_DECLARE_METATYPE(IntPairList)
Q_DECLARE_METATYPE(QModelIndex)

#endif // QT_VERSION

class tst_QSortFilterProxyModel : public QObject
{
    Q_OBJECT

public:

    tst_QSortFilterProxyModel();
    virtual ~tst_QSortFilterProxyModel();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void getSetCheck();
    void sort_data();
    void sort();
    void sortHierarchy_data();
    void sortHierarchy();
    
    void insertRows_data();
    void insertRows();
//     void insertColumns_data();
//     void insertColumns();
    void removeRows_data();
    void removeRows();
    void removeColumns_data();
    void removeColumns();
    void insertAfterSelect();
    void removeAfterSelect();
    void filter_data();
    void filter();
    void filterHierarchy_data();
    void filterHierarchy();

    void filterTable();
//    void filterCurrent();

#if QT_VERSION >= 0x040200
    void changeSourceLayout();
    void removeSourceRows_data();
    void removeSourceRows();
    void insertSourceRows_data();
    void insertSourceRows();
    void changeFilter_data();
    void changeFilter();
    void changeSourceData_data();
    void changeSourceData();
    void sortFilterRole();
    void selectionFilteredOut();
    void match_data();
    void match();
    void insertIntoChildrenlessItem();
    void invalidateMappedChildren();
    void insertRowIntoFilteredParent();
    void filterOutParentAndFilterInChild();
#endif // QT_VERSION

protected:
    void buildHierarchy(const QStringList &data, QAbstractItemModel *model);
    void checkHierarchy(const QStringList &data, const QAbstractItemModel *model);

private:
    QStandardItemModel *m_model;
    QSortFilterProxyModel *m_proxy;
};

// Testing get/set functions
void tst_QSortFilterProxyModel::getSetCheck()
{
    QSortFilterProxyModel obj1;
    QCOMPARE(obj1.sourceModel(), (const QAbstractItemModel *)0);
    // int QSortFilterProxyModel::filterKeyColumn()
    // void QSortFilterProxyModel::setFilterKeyColumn(int)
    obj1.setFilterKeyColumn(0);
    QCOMPARE(0, obj1.filterKeyColumn());
    obj1.setFilterKeyColumn(INT_MIN);
    QCOMPARE(INT_MIN, obj1.filterKeyColumn());
    obj1.setFilterKeyColumn(INT_MAX);
    QCOMPARE(INT_MAX, obj1.filterKeyColumn());
}

tst_QSortFilterProxyModel::tst_QSortFilterProxyModel()
    : m_model(0), m_proxy(0)
{

}

tst_QSortFilterProxyModel::~tst_QSortFilterProxyModel()
{

}

void tst_QSortFilterProxyModel::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
    qRegisterMetaType<IntList>("IntList");
    qRegisterMetaType<IntPair>("IntPair");
    qRegisterMetaType<IntPairList>("IntPairList");
    m_model = new QStandardItemModel(0, 1);
    m_proxy = new QSortFilterProxyModel();
    m_proxy->setSourceModel(m_model);
}

void tst_QSortFilterProxyModel::cleanupTestCase()
{
    delete m_proxy;
    delete m_model;
}

void tst_QSortFilterProxyModel::init()
{
}

void tst_QSortFilterProxyModel::cleanup()
{
    m_proxy->setFilterRegExp(QRegExp());
    m_proxy->sort(-1, Qt::AscendingOrder);
    m_model->clear();
    m_model->insertColumns(0, 1);
}

/*
  tests
*/

void tst_QSortFilterProxyModel::sort_data()
{
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<int>("sortCaseSensitivity");
    QTest::addColumn<QStringList>("initial");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("flat descending") << static_cast<int>(Qt::DescendingOrder)
                                  << int(Qt::CaseSensitive)
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
                                 << int(Qt::CaseSensitive)
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
#if QT_VERSION >= 0x040200
    QTest::newRow("case insensitive") <<  static_cast<int>(Qt::AscendingOrder)
                                 << int(Qt::CaseInsensitive)
                                 << (QStringList()
                                     << "alpha" << "BETA" << "Gamma" << "delta")
                                 << (QStringList()
                                     << "alpha" << "BETA" << "delta" << "Gamma");
#endif
    QTest::newRow("case sensitive") <<  static_cast<int>(Qt::AscendingOrder)
                                 << int(Qt::CaseSensitive)
                                 << (QStringList()
                                     << "alpha" << "BETA" << "Gamma" << "delta")
                                 << (QStringList()
                                     << "BETA" << "Gamma" << "alpha" << "delta");


    QStringList list;
    for (int i=100000;i < 200000; ++i)
        list.append(QString("Number: %1").arg(i));
    QTest::newRow("large set ascending") <<  static_cast<int>(Qt::AscendingOrder) << int(Qt::CaseSensitive) << list << list;
}

void tst_QSortFilterProxyModel::sort()
{
    QFETCH(int, sortOrder);
    QFETCH(int, sortCaseSensitivity);
    QFETCH(QStringList, initial);
    QFETCH(QStringList, expected);
    // prepare model
    QVERIFY(m_model->insertRows(0, initial.count(), QModelIndex()));
    QCOMPARE(m_model->rowCount(QModelIndex()), initial.count());
    //m_model->insertColumns(0, 1, QModelIndex());
    QCOMPARE(m_model->columnCount(QModelIndex()), 1);
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_model->index(row, 0, QModelIndex());
        m_model->setData(index, initial.at(row), Qt::DisplayRole);
    }
    // make sure the proxy is unsorted
    QCOMPARE(m_proxy->columnCount(QModelIndex()), 1);
    QCOMPARE(m_proxy->rowCount(QModelIndex()), initial.count());

//    qDebug() << proxy->rowCount(QModelIndex()) << model->rowCount(QModelIndex());
    
    for (int row = 0; row < m_proxy->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_proxy->index(row, 0, QModelIndex());
        QCOMPARE(m_proxy->data(index, Qt::DisplayRole).toString(), initial.at(row));
//        qDebug() << "*" << m_proxy->data(index, Qt::DisplayRole).toString();
    }
    // sort
    m_proxy->sort(0, static_cast<Qt::SortOrder>(sortOrder));
#if QT_VERSION >= 0x040200
    m_proxy->setSortCaseSensitivity(static_cast<Qt::CaseSensitivity>(sortCaseSensitivity));
#endif

    // make sure the model is unchanged
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_model->index(row, 0, QModelIndex());
        QCOMPARE(m_model->data(index, Qt::DisplayRole).toString(), initial.at(row));
    }
    // make sure the proxy is sorted
    for (int row = 0; row < m_proxy->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_proxy->index(row, 0, QModelIndex());
        QCOMPARE(m_proxy->data(index, Qt::DisplayRole).toString(), expected.at(row));
    }
}

void tst_QSortFilterProxyModel::sortHierarchy_data()
{
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<QStringList>("initial");
    QTest::addColumn<QStringList>("expected");

#if 1
    QTest::newRow("flat ascending")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList()
            << "c" << "f" << "d" << "e" << "a" << "b")
        << (QStringList()
            << "a" << "b" << "c" << "d" << "e" << "f");
#endif
    QTest::newRow("simple hierarchy")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList() << "a" << "<" << "b" << "<" << "c" << ">" << ">")
        << (QStringList() << "a" << "<" << "b" << "<" << "c" << ">" << ">");
    
#if 1
    QTest::newRow("hierarchical ascending")
        << static_cast<int>(Qt::AscendingOrder)
        << (QStringList()
            << "c"
                   << "<"
                   << "h"
                          << "<"
                          << "2"
                          << "0"
                          << "1"
                          << ">"
                   << "g"
                   << "i"
                   << ">"
            << "b"
                   << "<"
                   << "l"
                   << "k"
                          << "<"
                          << "8"
                          << "7"
                          << "9"
                          << ">"
                   << "m"
                   << ">"
            << "a"
                   << "<"
                   << "z"
                   << "y"
                   << "x"
                   << ">")
        << (QStringList()
            << "a"
                   << "<"
                   << "x"
                   << "y"
                   << "z"
                   << ">"
            << "b"
                   << "<"
                   << "k"
                          << "<"
                          << "7"
                          << "8"
                          << "9"
                          << ">"
                   << "l"
                   << "m"
                   << ">"
            << "c"
                   << "<"
                   << "g"
                   << "h"
                          << "<"
                          << "0"
                          << "1"
                          << "2"
                          << ">"
                   << "i"
                   << ">");
#endif
}

void tst_QSortFilterProxyModel::sortHierarchy()
{
    QFETCH(int, sortOrder);
    QFETCH(QStringList, initial);
    QFETCH(QStringList, expected);

    buildHierarchy(initial, m_model);
    checkHierarchy(initial, m_model);
    checkHierarchy(initial, m_proxy);
    m_proxy->sort(0, static_cast<Qt::SortOrder>(sortOrder));
    checkHierarchy(initial, m_model);
    checkHierarchy(expected, m_proxy);
}

void tst_QSortFilterProxyModel::insertRows_data()
{
    QTest::addColumn<QStringList>("initial");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QStringList>("insert");
    QTest::addColumn<int>("position");

    QTest::newRow("insert one row in the middle")
        << (QStringList()
            << "One"
            << "Two"
            << "Four"
            << "Five")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << (QStringList()
            << "Three")
        << 2;

    QTest::newRow("insert one row in the begining")
        << (QStringList()
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << (QStringList()
            << "One")
        << 0;

    QTest::newRow("insert one row in the end")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << (QStringList()
            <<"Five")
        << 4;
}

void tst_QSortFilterProxyModel::insertRows()
{
    QFETCH(QStringList, initial);
    QFETCH(QStringList, expected);
    QFETCH(QStringList, insert);
    QFETCH(int, position);
    // prepare model
    m_model->insertRows(0, initial.count(), QModelIndex());
    //m_model->insertColumns(0, 1, QModelIndex());
    QCOMPARE(m_model->columnCount(QModelIndex()), 1);
    QCOMPARE(m_model->rowCount(QModelIndex()), initial.count());
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_model->index(row, 0, QModelIndex());
        m_model->setData(index, initial.at(row), Qt::DisplayRole);
    }
    // make sure the model correct before insert
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_model->index(row, 0, QModelIndex());
        QCOMPARE(m_model->data(index, Qt::DisplayRole).toString(), initial.at(row));
    }
    // make sure the proxy is correct before insert
    for (int row = 0; row < m_proxy->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_proxy->index(row, 0, QModelIndex());
        QCOMPARE(m_proxy->data(index, Qt::DisplayRole).toString(), initial.at(row));
    }
    
    // insert the row
    m_proxy->insertRows(position, insert.count(), QModelIndex());
    QCOMPARE(m_model->rowCount(QModelIndex()), expected.count());
    QCOMPARE(m_proxy->rowCount(QModelIndex()), expected.count());

    // set the data for the inserted row
    for (int i = 0; i < insert.count(); ++i) {
        QModelIndex index = m_proxy->index(position + i, 0, QModelIndex());
        m_proxy->setData(index, insert.at(i), Qt::DisplayRole);
     }

    // make sure the model correct after insert
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_model->index(row, 0, QModelIndex());
        QCOMPARE(m_model->data(index, Qt::DisplayRole).toString(), expected.at(row));
    }

    // make sure the proxy is correct after insert
    for (int row = 0; row < m_proxy->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_proxy->index(row, 0, QModelIndex());
        QCOMPARE(m_proxy->data(index, Qt::DisplayRole).toString(), expected.at(row));
    }
}

/*
void tst_QSortFilterProxyModel::insertColumns_data()
{
    
}

void tst_QSortFilterProxyModel::insertColumns()
{

}
*/

void tst_QSortFilterProxyModel::removeRows_data()
{
    QTest::addColumn<QStringList>("initial");
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<QString>("filter");
    QTest::addColumn<int>("position");
    QTest::addColumn<int>("count");
    QTest::addColumn<bool>("success");
    QTest::addColumn<QStringList>("expectedProxy");
    QTest::addColumn<QStringList>("expectedSource");

    QTest::newRow("remove one row in the middle [no sorting/filter]")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << -1 // no sorting
        << QString() // no filter
        << 2 // position
        << 1 // count
        << true // success
        << (QStringList() // expectedProxy
            << "One"
            << "Two"
            << "Four"
            << "Five")
        << (QStringList() // expectedSource
            << "One"
            << "Two"
            << "Four"
            << "Five");

    QTest::newRow("remove one row in the beginning [no sorting/filter]")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << -1 // no sorting
        << QString() // no filter
        << 0 // position
        << 1 // count
        << true // success
        << (QStringList() // expectedProxy
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << (QStringList() // expectedSource
            << "Two"
            << "Three"
            << "Four"
            << "Five");

    QTest::newRow("remove one row in the end [no sorting/filter]")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << -1 // no sorting
        << QString() // no filter
        << 4 // position
        << 1 // count
        << true // success
        << (QStringList() // expectedProxy
            << "One"
            << "Two"
            << "Three"
            << "Four")
        << (QStringList() // expectedSource
            << "One"
            << "Two"
            << "Three"
            << "Four");

    QTest::newRow("remove all [no sorting/filter]")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << -1 // no sorting
        << QString() // no filter
        << 0 // position
        << 5 // count
        << true // success
        << QStringList() // expectedProxy
        << QStringList(); // expectedSource

    QTest::newRow("remove one row past the end [no sorting/filter]")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << -1 // no sorting
        << QString() // no filter
        << 5 // position
        << 1 // count
        << false // success
        << (QStringList() // expectedProxy
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << (QStringList() // expectedSource
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five");

    QTest::newRow("remove row -1 [no sorting/filter]")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << -1 // no sorting
        << QString() // no filter
        << -1 // position
        << 1 // count
        << false // success
        << (QStringList() // expectedProxy
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << (QStringList() // expectedSource
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five");

    QTest::newRow("remove three rows in the middle [no sorting/filter]")
        << (QStringList()
            << "One"
            << "Two"
            << "Three"
            << "Four"
            << "Five")
        << -1 // no sorting
        << QString() // no filter
        << 1 // position
        << 3 // count
        << true // success
        << (QStringList() // expectedProxy
            << "One"
            << "Five")
        << (QStringList() // expectedSource
            << "One"
            << "Five");

    QTest::newRow("remove one row in the middle [ascending sorting, no filter]")
        << (QStringList()
            << "1"
            << "5"
            << "2"
            << "4"
            << "3")
        << static_cast<int>(Qt::AscendingOrder)
        << QString() // no filter
        << 2 // position
        << 1 // count
        << true // success
        << (QStringList() // expectedProxy
            << "1"
            << "2"
            << "4"
            << "5")
        << (QStringList() // expectedSource
            << "1"
            << "5"
            << "2"
            << "4");

    QTest::newRow("remove two rows in the middle [ascending sorting, no filter]")
        << (QStringList()
            << "1"
            << "5"
            << "2"
            << "4"
            << "3")
        << static_cast<int>(Qt::AscendingOrder)
        << QString() // no filter
        << 2 // position
        << 2 // count
        << true // success
        << (QStringList() // expectedProxy
            << "1"
            << "2"
            << "5")
        << (QStringList() // expectedSource
            << "1"
            << "5"
            << "2");

    QTest::newRow("remove two rows in the middle [descending sorting, no filter]")
        << (QStringList()
            << "1"
            << "5"
            << "2"
            << "4"
            << "3")
        << static_cast<int>(Qt::DescendingOrder)
        << QString() // no filter
        << 2 // position
        << 2 // count
        << true // success
        << (QStringList() // expectedProxy
            << "5"
            << "4"
            << "1")
        << (QStringList() // expectedSource
            << "1"
            << "5"
            << "4");

    QTest::newRow("remove one row in the middle [no sorting, filter=5|2|3]")
        << (QStringList()
            << "1"
            << "5"
            << "2"
            << "4"
            << "3")
        << -1 // no sorting
        << QString("5|2|3")
        << 1 // position
        << 1 // count
        << true // success
        << (QStringList() // expectedProxy
            << "5"
            << "3")
        << (QStringList() // expectedSource
            << "1"
            << "5"
            << "4"
            << "3");

    QTest::newRow("remove all [ascending sorting, no filter]")
        << (QStringList()
            << "1"
            << "5"
            << "2"
            << "4"
            << "3")
        << static_cast<int>(Qt::AscendingOrder)
        << QString() // no filter
        << 0 // position
        << 5 // count
        << true // success
        << QStringList() // expectedProxy
        << QStringList(); // expectedSource
}

void tst_QSortFilterProxyModel::removeRows()
{
    QFETCH(QStringList, initial);
    QFETCH(int, sortOrder);
    QFETCH(QString, filter);
    QFETCH(int, position);
    QFETCH(int, count);
    QFETCH(bool, success);
    QFETCH(QStringList, expectedProxy);
    QFETCH(QStringList, expectedSource);

    QStandardItemModel model;
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    if (sortOrder != -1)
        proxy.sort(0, static_cast<Qt::SortOrder>(sortOrder));
    if (!filter.isEmpty())
        proxy.setFilterRegExp(QRegExp(filter));

    // prepare model
    foreach (QString s, initial)
        model.appendRow(new QStandardItem(s));
    
    // remove the rows
    QCOMPARE(proxy.removeRows(position, count, QModelIndex()), success);
    QCOMPARE(model.rowCount(QModelIndex()), expectedSource.count());
    QCOMPARE(proxy.rowCount(QModelIndex()), expectedProxy.count());

    // make sure the model is correct after remove
    for (int row = 0; row < model.rowCount(QModelIndex()); ++row)
        QCOMPARE(model.item(row)->text(), expectedSource.at(row));

    // make sure the proxy is correct after remove
    for (int row = 0; row < proxy.rowCount(QModelIndex()); ++row) {
        QModelIndex index = proxy.index(row, 0, QModelIndex());
        QCOMPARE(proxy.data(index, Qt::DisplayRole).toString(), expectedProxy.at(row));
    }
}

class MyFilteredColumnProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MyFilteredColumnProxyModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent) { }
protected:
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &) const
    {
        QString key = sourceModel()->headerData(sourceColumn, Qt::Horizontal).toString();
        return key.contains(filterRegExp());
    }
};

void tst_QSortFilterProxyModel::removeColumns_data()
{
    QTest::addColumn<QStringList>("initial");
    QTest::addColumn<QString>("filter");
    QTest::addColumn<int>("position");
    QTest::addColumn<int>("count");
    QTest::addColumn<bool>("success");
    QTest::addColumn<QStringList>("expectedProxy");
    QTest::addColumn<QStringList>("expectedSource");

    QTest::newRow("remove one column in the middle [no filter]")
        << (QStringList()
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << QString() // no filter
        << 2 // position
        << 1 // count
        << true // success
        << (QStringList() // expectedProxy
            << "1"
            << "2"
            << "4"
            << "5")
        << (QStringList() // expectedSource
            << "1"
            << "2"
            << "4"
            << "5");

    QTest::newRow("remove one column in the end [no filter]")
        << (QStringList()
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << QString() // no filter
        << 4 // position
        << 1 // count
        << true // success
        << (QStringList() // expectedProxy
            << "1"
            << "2"
            << "3"
            << "4")
        << (QStringList() // expectedSource
            << "1"
            << "2"
            << "3"
            << "4");

    QTest::newRow("remove one column past the end [no filter]")
        << (QStringList()
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << QString() // no filter
        << 5 // position
        << 1 // count
        << false // success
        << (QStringList() // expectedProxy
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << (QStringList() // expectedSource
            << "1"
            << "2"
            << "3"
            << "4"
            << "5");

    QTest::newRow("remove column -1 [no filter]")
        << (QStringList()
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << QString() // no filter
        << -1 // position
        << 1 // count
        << false // success
        << (QStringList() // expectedProxy
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << (QStringList() // expectedSource
            << "1"
            << "2"
            << "3"
            << "4"
            << "5");

    QTest::newRow("remove all columns [no filter]")
        << (QStringList()
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << QString() // no filter
        << 0 // position
        << 5 // count
        << true // success
        << QStringList() // expectedProxy
        << QStringList(); // expectedSource

    QTest::newRow("remove one column in the middle [filter=1|3|5]")
        << (QStringList()
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << QString("1|3|5")
        << 1 // position
        << 1 // count
        << true // success
        << (QStringList() // expectedProxy
            << "1"
            << "5")
        << (QStringList() // expectedSource
            << "1"
            << "2"
            << "4"
            << "5");

    QTest::newRow("remove one column in the end [filter=1|3|5]")
        << (QStringList()
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << QString("1|3|5")
        << 2 // position
        << 1 // count
        << true // success
        << (QStringList() // expectedProxy
            << "1"
            << "3")
        << (QStringList() // expectedSource
            << "1"
            << "2"
            << "3"
            << "4");

    QTest::newRow("remove one column past the end [filter=1|3|5]")
        << (QStringList()
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << QString("1|3|5")
        << 3 // position
        << 1 // count
        << false // success
        << (QStringList() // expectedProxy
            << "1"
            << "3"
            << "5")
        << (QStringList() // expectedSource
            << "1"
            << "2"
            << "3"
            << "4"
            << "5");

    QTest::newRow("remove all columns [filter=1|3|5]")
        << (QStringList()
            << "1"
            << "2"
            << "3"
            << "4"
            << "5")
        << QString("1|3|5")
        << 0 // position
        << 3 // count
        << true // success
        << QStringList() // expectedProxy
        << (QStringList() // expectedSource
            << "2"
            << "4");
}

void tst_QSortFilterProxyModel::removeColumns()
{
    QFETCH(QStringList, initial);
    QFETCH(QString, filter);
    QFETCH(int, position);
    QFETCH(int, count);
    QFETCH(bool, success);
    QFETCH(QStringList, expectedProxy);
    QFETCH(QStringList, expectedSource);

    QStandardItemModel model;
    MyFilteredColumnProxyModel proxy;
    proxy.setSourceModel(&model);
    if (!filter.isEmpty())
        proxy.setFilterRegExp(QRegExp(filter));
    
    // prepare model
    model.setHorizontalHeaderLabels(initial);
    
    // remove the columns
    QCOMPARE(proxy.removeColumns(position, count, QModelIndex()), success);
    QCOMPARE(model.columnCount(QModelIndex()), expectedSource.count());
    QCOMPARE(proxy.columnCount(QModelIndex()), expectedProxy.count());

    // make sure the model is correct after remove
    for (int col = 0; col < model.columnCount(QModelIndex()); ++col)
        QCOMPARE(model.horizontalHeaderItem(col)->text(), expectedSource.at(col));

    // make sure the proxy is correct after remove
    for (int col = 0; col < proxy.columnCount(QModelIndex()); ++col) {
        QCOMPARE(proxy.headerData(col, Qt::Horizontal, Qt::DisplayRole).toString(),
                 expectedProxy.at(col));
    }
}

void tst_QSortFilterProxyModel::filter_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QStringList>("initial");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("flat") << "e"
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
                           << "delta"
                           << "yankee"
                           << "charlie"
                           << "juliet"
                           << "hotel"
                           << "echo"
                           << "quebec"
                           << "romeo"
                           << "november"
                           << "whiskey"
                           << "mike"
                           << "sierra");
}

void tst_QSortFilterProxyModel::filter()
{
    QFETCH(QString, pattern);
    QFETCH(QStringList, initial);
    QFETCH(QStringList, expected);
    // prepare model
    QVERIFY(m_model->insertRows(0, initial.count(), QModelIndex()));
    QCOMPARE(m_model->rowCount(QModelIndex()), initial.count());
    // set data
    QCOMPARE(m_model->columnCount(QModelIndex()), 1);
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_model->index(row, 0, QModelIndex());
        m_model->setData(index, initial.at(row), Qt::DisplayRole);
    }
    m_proxy->setFilterRegExp(pattern);
    // make sure the proxy is unfiltered
    QCOMPARE(m_proxy->columnCount(QModelIndex()), 1);
    QCOMPARE(m_proxy->rowCount(QModelIndex()), expected.count());
    // make sure the model is unchanged
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_model->index(row, 0, QModelIndex());
        QCOMPARE(m_model->data(index, Qt::DisplayRole).toString(), initial.at(row));
    }
    // make sure the proxy is filtered
    for (int row = 0; row < m_proxy->rowCount(QModelIndex()); ++row) {
        QModelIndex index = m_proxy->index(row, 0, QModelIndex());
        QCOMPARE(m_proxy->data(index, Qt::DisplayRole).toString(), expected.at(row));
    }
}

void tst_QSortFilterProxyModel::filterHierarchy_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QStringList>("initial");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("flat") << ".*oo"
        << (QStringList()
            << "foo" << "boo" << "baz" << "moo" << "laa" << "haa")
        << (QStringList()
            << "foo" << "boo" << "moo");
    
    QTest::newRow("simple hierarchy") << "b.*z"
        << (QStringList() << "baz" << "<" << "boz" << "<" << "moo" << ">" << ">")
        << (QStringList() << "baz" << "<" << "boz" << ">");
#if 0
    QTest::newRow("hierarchical")
        << (QStringList()
            << "a"
                   << "<"
                   << "x"
                   << "y"
                   << "z"
                   << ">"
            << "b"
                   << "<"
                   << "k"
                          << "<"
                          << "7"
                          << "8"
                          << "9"
                          << ">"
                   << "l"
                   << "m"
                   << ">"
            << "c"
                   << "<"
                   << "g"
                   << "h"
                          << "<"
                          << "0"
                          << "1"
                          << "2"
                          << ">"
                   << "i"
                   << ">")
        << (QStringList()
            << "a"
                   << "<"
                   << "x"
                   << "z"
                   << ">"
            << "c"
                   << "<"
                   << "g"
                   << "i"
                   << ">");
#endif
}

void tst_QSortFilterProxyModel::filterHierarchy()
{
    QFETCH(QString, pattern);
    QFETCH(QStringList, initial);
    QFETCH(QStringList, expected);
    buildHierarchy(initial, m_model);
    m_proxy->setFilterRegExp(pattern);
    checkHierarchy(initial, m_model);
    checkHierarchy(expected, m_proxy);
}

void tst_QSortFilterProxyModel::buildHierarchy(const QStringList &l, QAbstractItemModel *m)
{
    int ind = 0;
    int row = 0;
    QStack<int> row_stack;
    QModelIndex parent;
    QStack<QModelIndex> parent_stack;
    for (int i = 0; i < l.count(); ++i) {
        QString token = l.at(i);
        if (token == "<") { // start table
            ++ind;
            parent_stack.push(parent);
            row_stack.push(row);
            parent = m->index(row - 1, 0, parent);
            row = 0;
            QVERIFY(m->insertColumns(0, 1, parent)); // add column
        } else if (token == ">") { // end table
            --ind;
            parent = parent_stack.pop();
            row = row_stack.pop();
        } else { // append row
            QVERIFY(m->insertRows(row, 1, parent));
            QModelIndex index = m->index(row, 0, parent);
            QVERIFY(index.isValid());
            m->setData(index, token, Qt::DisplayRole);
            ++row;
#if 0
            {
                QString txt = token;
                for (int t = 0; t < ind; ++t)
                    txt.prepend(' ');
                qDebug() << "#" << txt;
            }
#endif
        }
    }
}

void tst_QSortFilterProxyModel::checkHierarchy(const QStringList &l, const QAbstractItemModel *m)
{
    int row = 0;
    int indent = 0;
    QStack<int> row_stack;
    QModelIndex parent;
    QStack<QModelIndex> parent_stack;
    for (int i = 0; i < l.count(); ++i) {
        QString token = l.at(i);
        if (token == "<") { // start table
            ++indent;
            parent_stack.push(parent);
            row_stack.push(row);
            parent = m->index(row - 1, 0, parent);
            QVERIFY(parent.isValid());
            row = 0;
        } else if (token == ">") { // end table
            --indent;
            parent = parent_stack.pop();
            row = row_stack.pop();
        } else { // compare row
            QModelIndex index = m->index(row, 0, parent);
            QVERIFY(index.isValid());
            QString str =  m->data(index, Qt::DisplayRole).toString();
#if 0
            qDebug() << "proxy data is" << str << "level" << indent;
#endif
            QCOMPARE(str, token);
            ++row;
        }
    }

}

class TestModel: public QAbstractTableModel
{
public:
    int rowCount(const QModelIndex &) const { return 10000; }
    int columnCount(const QModelIndex &) const { return 1; }
    QVariant data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();
        return QString::number(index.row());
    }
};

void tst_QSortFilterProxyModel::filterTable()
{
    TestModel model;
    QSortFilterProxyModel filter;
    filter.setSourceModel(&model);
    filter.setFilterRegExp("9");

    for (int i = 0; i < filter.rowCount(); ++i)
        QVERIFY(filter.data(filter.index(i, 0)).toString().contains("9"));
}

void tst_QSortFilterProxyModel::insertAfterSelect()
{
    QStandardItemModel model(10, 2);
    for (int i = 0; i<10;i++)
        model.setData(model.index(i, 0), QVariant(i));
    QSortFilterProxyModel filter;
    filter.setSourceModel(&model);
    QTreeView view;
    view.setModel(&filter);
    view.show();
    QModelIndex firstIndex = model.index(0, 0, QModelIndex());
    QVERIFY(firstIndex.isValid());
    int itemOffset = view.visualRect(firstIndex).width() / 2;
    QPoint p(itemOffset, 1);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    QVERIFY(view.selectionModel()->selectedIndexes().size() > 0);
    model.insertRows(5, 1, QModelIndex());
#if QT_VERSION < 0x040200
    QEXPECT_FAIL("", "Selections are not kept in versions < 4.2", Abort);
#endif
    QVERIFY(view.selectionModel()->selectedIndexes().size() > 0); // Should still have a selection
}

void tst_QSortFilterProxyModel::removeAfterSelect()
{
    QStandardItemModel model(10, 2);
    for (int i = 0; i<10;i++)
        model.setData(model.index(i, 0), QVariant(i));
    QSortFilterProxyModel filter;
    filter.setSourceModel(&model);
    QTreeView view;
    view.setModel(&filter);
    view.show();
    QModelIndex firstIndex = model.index(0, 0, QModelIndex());
    QVERIFY(firstIndex.isValid());
    int itemOffset = view.visualRect(firstIndex).width() / 2;
    QPoint p(itemOffset, 1);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    QVERIFY(view.selectionModel()->selectedIndexes().size() > 0);
    model.removeRows(5, 1, QModelIndex());
#if QT_VERSION < 0x040200
    QEXPECT_FAIL("", "Selections are not kept in versions < 4.2", Abort);
#endif
    QVERIFY(view.selectionModel()->selectedIndexes().size() > 0); // Should still have a selection
}

#if 0 // this test is disabled for now
void tst_QSortFilterProxyModel::filterCurrent()
{
    QStandardItemModel model(2, 1);
    model.setData(model.index(0, 0), QString("AAA"));
    model.setData(model.index(1, 0), QString("BBB"));
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    QTreeView view;

    view.show();
    view.setModel(&proxy);
    QSignalSpy spy(view.selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)));

    view.setCurrentIndex(proxy.index(0, 0));
    QCOMPARE(spy.count(), 1);
    proxy.setFilterRegExp(QRegExp("^B"));
    QCOMPARE(spy.count(), 2);
}
#endif

#if QT_VERSION >= 0x040200

void tst_QSortFilterProxyModel::changeSourceLayout()
{
    QStandardItemModel model(2, 1);
    model.setData(model.index(0, 0), QString("b"));
    model.setData(model.index(1, 0), QString("a"));
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);

    QList<QPersistentModelIndex> persistentSourceIndexes;
    QList<QPersistentModelIndex> persistentProxyIndexes;
    for (int row = 0; row < model.rowCount(); ++row) {
        persistentSourceIndexes.append(model.index(row, 0));
        persistentProxyIndexes.append(proxy.index(row, 0));
    }

    // change layout of source model
    model.sort(0, Qt::AscendingOrder);

    for (int row = 0; row < model.rowCount(); ++row) {
        QCOMPARE(persistentProxyIndexes.at(row).row(),
                 persistentSourceIndexes.at(row).row());
    }
}

void tst_QSortFilterProxyModel::removeSourceRows_data()
{
    QTest::addColumn<QStringList>("sourceItems");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<IntPairList>("expectedRemovedProxyIntervals");
    QTest::addColumn<QStringList>("expectedProxyItems");

    QTest::newRow("remove one, no sorting")
        << (QStringList() << "a" << "b") // sourceItems
        << 0 // start
        << 1 // count
        << -1 // sortOrder (no sorting)
        << (IntPairList() << IntPair(0, 0)) // expectedRemovedIntervals
        << (QStringList() << "b") // expectedProxyItems
        ;
    QTest::newRow("remove one, ascending sort (same order)")
        << (QStringList() << "a" << "b") // sourceItems
        << 0 // start
        << 1 // count
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << (IntPairList() << IntPair(0, 0)) // expectedRemovedIntervals
        << (QStringList() << "b") // expectedProxyItems
        ;
    QTest::newRow("remove one, ascending sort (reverse order)")
        << (QStringList() << "b" << "a") // sourceItems
        << 0 // start
        << 1 // count
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << (IntPairList() << IntPair(1, 1)) // expectedRemovedIntervals
        << (QStringList() << "a") // expectedProxyItems
        ;
    QTest::newRow("remove two, multiple proxy intervals")
        << (QStringList() << "c" << "d" << "a" << "b") // sourceItems
        << 1 // start
        << 2 // count
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << (IntPairList() << IntPair(3, 3) << IntPair(0, 0)) // expectedRemovedIntervals
        << (QStringList() << "b" << "c") // expectedProxyItems
        ;
    QTest::newRow("remove three, multiple proxy intervals")
        << (QStringList() << "b" << "d" << "f" << "a" << "c" << "e") // sourceItems
        << 3 // start
        << 3 // count
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << (IntPairList() << IntPair(4, 4) << IntPair(2, 2) << IntPair(0, 0)) // expectedRemovedIntervals
        << (QStringList() << "b" << "d" << "f") // expectedProxyItems
        ;
    QTest::newRow("remove all, single proxy intervals")
        << (QStringList() << "a" << "b" << "c" << "d" << "e" << "f") // sourceItems
        << 0 // start
        << 6 // count
        << static_cast<int>(Qt::DescendingOrder) // sortOrder
        << (IntPairList() << IntPair(0, 5)) // expectedRemovedIntervals
        << QStringList() // expectedProxyItems
        ;
}

// Check that correct proxy model rows are removed when rows are removed
// from the source model
void tst_QSortFilterProxyModel::removeSourceRows()
{
    QFETCH(QStringList, sourceItems);
    QFETCH(int, start);
    QFETCH(int, count);
    QFETCH(int, sortOrder);
    QFETCH(IntPairList, expectedRemovedProxyIntervals);
    QFETCH(QStringList, expectedProxyItems);

    QStandardItemModel model;
    QSortFilterProxyModel proxy;

    proxy.setSourceModel(&model);
    model.insertColumns(0, 1);
    model.insertRows(0, sourceItems.count());

    for (int i = 0; i < sourceItems.count(); ++i) {
        QModelIndex sindex = model.index(i, 0, QModelIndex());
        model.setData(sindex, sourceItems.at(i), Qt::DisplayRole);
        QModelIndex pindex = proxy.index(i, 0, QModelIndex());
        QCOMPARE(proxy.data(pindex, Qt::DisplayRole), model.data(sindex, Qt::DisplayRole));
    }

    if (sortOrder != -1)
        proxy.sort(0, static_cast<Qt::SortOrder>(sortOrder));
    (void)proxy.rowCount(QModelIndex()); // force mapping

    QSignalSpy removeSpy(&proxy, SIGNAL(rowsRemoved(QModelIndex, int, int)));
    QSignalSpy insertSpy(&proxy, SIGNAL(rowsInserted(QModelIndex, int, int)));
    QSignalSpy aboutToRemoveSpy(&proxy, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)));
    QSignalSpy aboutToInsertSpy(&proxy, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
    
    model.removeRows(start, count, QModelIndex());

    QCOMPARE(aboutToRemoveSpy.count(), expectedRemovedProxyIntervals.count());
    for (int i = 0; i < aboutToRemoveSpy.count(); ++i) {
        QList<QVariant> args = aboutToRemoveSpy.at(i);
        QVERIFY(args.at(1).type() == QVariant::Int);
        QVERIFY(args.at(2).type() == QVariant::Int);
        QCOMPARE(args.at(1).toInt(), expectedRemovedProxyIntervals.at(i).first);
        QCOMPARE(args.at(2).toInt(), expectedRemovedProxyIntervals.at(i).second);
    }
    QCOMPARE(removeSpy.count(), expectedRemovedProxyIntervals.count());
    for (int i = 0; i < removeSpy.count(); ++i) {
        QList<QVariant> args = removeSpy.at(i);
        QVERIFY(args.at(1).type() == QVariant::Int);
        QVERIFY(args.at(2).type() == QVariant::Int);
        QCOMPARE(args.at(1).toInt(), expectedRemovedProxyIntervals.at(i).first);
        QCOMPARE(args.at(2).toInt(), expectedRemovedProxyIntervals.at(i).second);
    }

    QCOMPARE(insertSpy.count(), 0);
    QCOMPARE(aboutToInsertSpy.count(), 0);

    QCOMPARE(proxy.rowCount(QModelIndex()), expectedProxyItems.count());
    for (int i = 0; i < expectedProxyItems.count(); ++i) {
        QModelIndex pindex = proxy.index(i, 0, QModelIndex());
        QCOMPARE(proxy.data(pindex, Qt::DisplayRole).toString(), expectedProxyItems.at(i));
    }
}

void tst_QSortFilterProxyModel::insertSourceRows_data()
{
    QTest::addColumn<QStringList>("sourceItems");
    QTest::addColumn<int>("start");
    QTest::addColumn<QStringList>("newItems");
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<QStringList>("proxyItems");

    QTest::newRow("insert (1)")
        << (QStringList() << "c" << "b") // sourceItems
        << 1 // start
        << (QStringList() << "a") // newItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << (QStringList() << "a" << "b" << "c") // proxyItems
        ;

    QTest::newRow("insert (2)")
        << (QStringList() << "d" << "b" << "c") // sourceItems
        << 3 // start
        << (QStringList() << "a") // newItems
        << static_cast<int>(Qt::DescendingOrder) // sortOrder
        << (QStringList() << "d" << "c" << "b" << "a") // proxyItems
        ;
}

// Check that rows are inserted at correct position in proxy model when
// rows are inserted into the source model
void tst_QSortFilterProxyModel::insertSourceRows()
{
    QFETCH(QStringList, sourceItems);
    QFETCH(int, start);
    QFETCH(QStringList, newItems);
    QFETCH(int, sortOrder);
    QFETCH(QStringList, proxyItems);

    QStandardItemModel model;
    QSortFilterProxyModel proxy;
    proxy.setDynamicSortFilter(true);

    proxy.setSourceModel(&model);
    model.insertColumns(0, 1);
    model.insertRows(0, sourceItems.count());

    for (int i = 0; i < sourceItems.count(); ++i) {
        QModelIndex index = model.index(i, 0, QModelIndex());
        model.setData(index, sourceItems.at(i), Qt::DisplayRole);
    }

    proxy.sort(0, static_cast<Qt::SortOrder>(sortOrder));
    (void)proxy.rowCount(QModelIndex()); // force mapping

    model.insertRows(start, newItems.size(), QModelIndex());

    QCOMPARE(proxy.rowCount(QModelIndex()), proxyItems.count());
    for (int i = 0; i < newItems.count(); ++i) {
        QModelIndex index = model.index(start + i, 0, QModelIndex());
        model.setData(index, newItems.at(i), Qt::DisplayRole);
    }

    for (int i = 0; i < proxyItems.count(); ++i) {
        QModelIndex index = proxy.index(i, 0, QModelIndex());
        QCOMPARE(proxy.data(index, Qt::DisplayRole).toString(), proxyItems.at(i));
    }
}

void tst_QSortFilterProxyModel::changeFilter_data()
{
    QTest::addColumn<QStringList>("sourceItems");
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<QString>("initialFilter");
    QTest::addColumn<IntPairList>("initialRemoveIntervals");
    QTest::addColumn<QStringList>("initialProxyItems");
    QTest::addColumn<QString>("finalFilter");
    QTest::addColumn<IntPairList>("finalRemoveIntervals");
    QTest::addColumn<IntPairList>("insertIntervals");
    QTest::addColumn<QStringList>("finalProxyItems");

    QTest::newRow("filter (1)")
        << (QStringList() << "a" << "b" << "c" << "d" << "e" << "f") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "a|b|c" // initialFilter
        << (IntPairList() << IntPair(3, 5)) // initialRemoveIntervals
        << (QStringList() << "a" << "b" << "c") // initialProxyItems
        << "b|d|f" // finalFilter
        << (IntPairList() << IntPair(2, 2) << IntPair(0, 0)) // finalRemoveIntervals
        << (IntPairList() << IntPair(1, 2)) // insertIntervals
        << (QStringList() << "b" << "d" << "f") // finalProxyItems
        ;

    QTest::newRow("filter (2)")
        << (QStringList() << "a" << "b" << "c" << "d" << "e" << "f") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "a|c|e" // initialFilter
        << (IntPairList() << IntPair(5, 5) << IntPair(3, 3) << IntPair(1, 1)) // initialRemoveIntervals
        << (QStringList() << "a" << "c" << "e") // initialProxyItems
        << "" // finalFilter
        << IntPairList() // finalRemoveIntervals
        << (IntPairList() << IntPair(3, 3) << IntPair(2, 2) << IntPair(1, 1)) // insertIntervals
        << (QStringList() << "a" << "b" << "c" << "d" << "e" << "f") // finalProxyItems
        ;

    QTest::newRow("filter (3)")
        << (QStringList() << "a" << "b" << "c") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "a" // initialFilter
        << (IntPairList() << IntPair(1, 2)) // initialRemoveIntervals
        << (QStringList() << "a") // initialProxyItems
        << "a" // finalFilter
        << IntPairList() // finalRemoveIntervals
        << IntPairList() // insertIntervals
        << (QStringList() << "a") // finalProxyItems
        ;
}

// Check that rows are added/removed when filter changes
void tst_QSortFilterProxyModel::changeFilter()
{
    QFETCH(QStringList, sourceItems);
    QFETCH(int, sortOrder);
    QFETCH(QString, initialFilter);
    QFETCH(IntPairList, initialRemoveIntervals);
    QFETCH(QStringList, initialProxyItems);
    QFETCH(QString, finalFilter);
    QFETCH(IntPairList, finalRemoveIntervals);
    QFETCH(IntPairList, insertIntervals);
    QFETCH(QStringList, finalProxyItems);

    QStandardItemModel model;
    QSortFilterProxyModel proxy;

    proxy.setSourceModel(&model);
    model.insertColumns(0, 1);
    model.insertRows(0, sourceItems.count());

    for (int i = 0; i < sourceItems.count(); ++i) {
        QModelIndex index = model.index(i, 0, QModelIndex());
        model.setData(index, sourceItems.at(i), Qt::DisplayRole);
    }

    proxy.sort(0, static_cast<Qt::SortOrder>(sortOrder));
    (void)proxy.rowCount(QModelIndex()); // force mapping

    QSignalSpy initialRemoveSpy(&proxy, SIGNAL(rowsRemoved(QModelIndex, int, int)));
    QSignalSpy initialInsertSpy(&proxy, SIGNAL(rowsInserted(QModelIndex, int, int)));

    proxy.setFilterRegExp(initialFilter);

    QCOMPARE(initialRemoveSpy.count(), initialRemoveIntervals.count());
    QCOMPARE(initialInsertSpy.count(), 0);
    for (int i = 0; i < initialRemoveSpy.count(); ++i) {
        QList<QVariant> args = initialRemoveSpy.at(i);
        QVERIFY(args.at(1).type() == QVariant::Int);
        QVERIFY(args.at(2).type() == QVariant::Int);
        QCOMPARE(args.at(1).toInt(), initialRemoveIntervals.at(i).first);
        QCOMPARE(args.at(2).toInt(), initialRemoveIntervals.at(i).second);
    }

    QCOMPARE(proxy.rowCount(QModelIndex()), initialProxyItems.count());
    for (int i = 0; i < initialProxyItems.count(); ++i) {
        QModelIndex index = proxy.index(i, 0, QModelIndex());
        QCOMPARE(proxy.data(index, Qt::DisplayRole).toString(), initialProxyItems.at(i));
    }

    QSignalSpy finalRemoveSpy(&proxy, SIGNAL(rowsRemoved(QModelIndex, int, int)));
    QSignalSpy finalInsertSpy(&proxy, SIGNAL(rowsInserted(QModelIndex, int, int)));

    proxy.setFilterRegExp(finalFilter);

    QCOMPARE(finalRemoveSpy.count(), finalRemoveIntervals.count());
    for (int i = 0; i < finalRemoveSpy.count(); ++i) {
        QList<QVariant> args = finalRemoveSpy.at(i);
        QVERIFY(args.at(1).type() == QVariant::Int);
        QVERIFY(args.at(2).type() == QVariant::Int);
        QCOMPARE(args.at(1).toInt(), finalRemoveIntervals.at(i).first);
        QCOMPARE(args.at(2).toInt(), finalRemoveIntervals.at(i).second);
    }

    QCOMPARE(finalInsertSpy.count(), insertIntervals.count());
    for (int i = 0; i < finalInsertSpy.count(); ++i) {
        QList<QVariant> args = finalInsertSpy.at(i);
        QVERIFY(args.at(1).type() == QVariant::Int);
        QVERIFY(args.at(2).type() == QVariant::Int);
        QCOMPARE(args.at(1).toInt(), insertIntervals.at(i).first);
        QCOMPARE(args.at(2).toInt(), insertIntervals.at(i).second);
    }

    QCOMPARE(proxy.rowCount(QModelIndex()), finalProxyItems.count());
    for (int i = 0; i < finalProxyItems.count(); ++i) {
        QModelIndex index = proxy.index(i, 0, QModelIndex());
        QCOMPARE(proxy.data(index, Qt::DisplayRole).toString(), finalProxyItems.at(i));
    }
}

void tst_QSortFilterProxyModel::changeSourceData_data()
{
    QTest::addColumn<QStringList>("sourceItems");
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<QString>("filter");
    QTest::addColumn<bool>("dynamic");
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("newValue");
    QTest::addColumn<IntPairList>("removeIntervals");
    QTest::addColumn<IntPairList>("insertIntervals");
    QTest::addColumn<QStringList>("proxyItems");

    QTest::newRow("changeSourceData (1)")
        << (QStringList() << "c" << "b" << "a") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "" // filter
        << true // dynamic
        << 2 // row
        << "z" // newValue
        << IntPairList() // removeIntervals
        << IntPairList() // insertIntervals
        << (QStringList() << "b" << "c" << "z") // proxyItems
        ;

    QTest::newRow("changeSourceData (2)")
        << (QStringList() << "b" << "c" << "z") // sourceItems
        << static_cast<int>(Qt::DescendingOrder) // sortOrder
        << "" // filter
        << true // dynamic
        << 1 // row
        << "a" // newValue
        << IntPairList() // removeIntervals
        << IntPairList() // insertIntervals
        << (QStringList() << "z" << "b" << "a") // proxyItems
        ;

    QTest::newRow("changeSourceData (3)")
        << (QStringList() << "a" << "b") // sourceItems
        << static_cast<int>(Qt::DescendingOrder) // sortOrder
        << "" // filter
        << true // dynamic
        << 0 // row
        << "a" // newValue
        << IntPairList() // removeIntervals
        << IntPairList() // insertIntervals
        << (QStringList() << "b" << "a") // proxyItems
        ;

    QTest::newRow("changeSourceData (4)")
        << (QStringList() << "a" << "b" << "c" << "d") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "a|c" // filter
        << true // dynamic
        << 1 // row
        << "x" // newValue
        << IntPairList() // removeIntervals
        << IntPairList() // insertIntervals
        << (QStringList() << "a" << "c") // proxyItems
        ;

    QTest::newRow("changeSourceData (5)")
        << (QStringList() << "a" << "b" << "c" << "d") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "a|c|x" // filter
        << true // dynamic
        << 1 // row
        << "x" // newValue
        << IntPairList() // removeIntervals
        << (IntPairList() << IntPair(2, 2)) // insertIntervals
        << (QStringList() << "a" << "c" << "x") // proxyItems
        ;

    QTest::newRow("changeSourceData (6)")
        << (QStringList() << "c" << "b" << "a") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "" // filter
        << false // dynamic
        << 2 // row
        << "x" // newValue
        << IntPairList() // removeIntervals
        << IntPairList() // insertIntervals
        << (QStringList() << "x" << "b" << "c") // proxyItems
        ;
}

void tst_QSortFilterProxyModel::changeSourceData()
{
    QFETCH(QStringList, sourceItems);
    QFETCH(int, sortOrder);
    QFETCH(QString, filter);
    QFETCH(bool, dynamic);
    QFETCH(int, row);
    QFETCH(QString, newValue);
    QFETCH(IntPairList, removeIntervals);
    QFETCH(IntPairList, insertIntervals);
    QFETCH(QStringList, proxyItems);

    QStandardItemModel model;
    QSortFilterProxyModel proxy;

    proxy.setDynamicSortFilter(dynamic);
    proxy.setSourceModel(&model);
    model.insertColumns(0, 1);
    model.insertRows(0, sourceItems.count());

    for (int i = 0; i < sourceItems.count(); ++i) {
        QModelIndex index = model.index(i, 0, QModelIndex());
        model.setData(index, sourceItems.at(i), Qt::DisplayRole);
    }

    proxy.sort(0, static_cast<Qt::SortOrder>(sortOrder));
    (void)proxy.rowCount(QModelIndex()); // force mapping

    proxy.setFilterRegExp(filter);

    QSignalSpy removeSpy(&proxy, SIGNAL(rowsRemoved(QModelIndex, int, int)));
    QSignalSpy insertSpy(&proxy, SIGNAL(rowsInserted(QModelIndex, int, int)));

    {
        QModelIndex index = model.index(row, 0, QModelIndex());
        model.setData(index, newValue, Qt::DisplayRole);
    }

    QCOMPARE(removeSpy.count(), removeIntervals.count());
    for (int i = 0; i < removeSpy.count(); ++i) {
        QList<QVariant> args = removeSpy.at(i);
        QVERIFY(args.at(1).type() == QVariant::Int);
        QVERIFY(args.at(2).type() == QVariant::Int);
        QCOMPARE(args.at(1).toInt(), removeIntervals.at(i).first);
        QCOMPARE(args.at(2).toInt(), removeIntervals.at(i).second);
    }

    QCOMPARE(insertSpy.count(), insertIntervals.count());
    for (int i = 0; i < insertSpy.count(); ++i) {
        QList<QVariant> args = insertSpy.at(i);
        QVERIFY(args.at(1).type() == QVariant::Int);
        QVERIFY(args.at(2).type() == QVariant::Int);
        QCOMPARE(args.at(1).toInt(), insertIntervals.at(i).first);
        QCOMPARE(args.at(2).toInt(), insertIntervals.at(i).second);
    }

    QCOMPARE(proxy.rowCount(QModelIndex()), proxyItems.count());
    for (int i = 0; i < proxyItems.count(); ++i) {
        QModelIndex index = proxy.index(i, 0, QModelIndex());
        QCOMPARE(proxy.data(index, Qt::DisplayRole).toString(), proxyItems.at(i));
    }
}

void tst_QSortFilterProxyModel::sortFilterRole()
{
    QStandardItemModel model;
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    model.insertColumns(0, 1);

    QList<QPair<QVariant, QVariant> > sourceItems;
    sourceItems = QList<QPair<QVariant, QVariant> >()
                  << QPair<QVariant, QVariant>("b", 3)
                  << QPair<QVariant, QVariant>("c", 2)
                  << QPair<QVariant, QVariant>("a", 1);

    QList<int> orderedItems;
    orderedItems = QList<int>()
                  << 2 << 1;

    model.insertRows(0, sourceItems.count());
    for (int i = 0; i < sourceItems.count(); ++i) {
        QModelIndex index = model.index(i, 0, QModelIndex());
        model.setData(index, sourceItems.at(i).first, Qt::DisplayRole);
        model.setData(index, sourceItems.at(i).second, Qt::UserRole);
    }
        
    proxy.setFilterRegExp("2");
    QCOMPARE(proxy.rowCount(), 0); // Qt::DisplayRole is default role

    proxy.setFilterRole(Qt::UserRole);
    QCOMPARE(proxy.rowCount(), 1);

    proxy.setFilterRole(Qt::DisplayRole);
    QCOMPARE(proxy.rowCount(), 0);

    proxy.setFilterRegExp("1|2|3");
    QCOMPARE(proxy.rowCount(), 0);

    proxy.setFilterRole(Qt::UserRole);
    QCOMPARE(proxy.rowCount(), 3);

    proxy.sort(0, Qt::AscendingOrder);
    QCOMPARE(proxy.rowCount(), 3);

    proxy.setSortRole(Qt::UserRole);
    proxy.setFilterRole(Qt::DisplayRole);
    proxy.setFilterRegExp("a|c");
    QCOMPARE(proxy.rowCount(), orderedItems.count());
    for (int i = 0; i < proxy.rowCount(); ++i) {
        QModelIndex index = proxy.index(i, 0, QModelIndex());
        QCOMPARE(proxy.data(index, Qt::DisplayRole), sourceItems.at(orderedItems.at(i)).first);
    }
}

void tst_QSortFilterProxyModel::selectionFilteredOut()
{
    QStandardItemModel model(2, 1);
    model.setData(model.index(0, 0), QString("AAA"));
    model.setData(model.index(1, 0), QString("BBB"));
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    QTreeView view;

    view.show();
    view.setModel(&proxy);
    QSignalSpy spy(view.selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)));

    view.setCurrentIndex(proxy.index(0, 0));
    QCOMPARE(spy.count(), 1);
    proxy.setFilterRegExp(QRegExp("^B"));
    QCOMPARE(spy.count(), 2);
}

void tst_QSortFilterProxyModel::match_data()
{
    QTest::addColumn<QStringList>("sourceItems");
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<QString>("filter");
    QTest::addColumn<int>("proxyStartRow");
    QTest::addColumn<QString>("what");
    QTest::addColumn<int>("matchFlags");
    QTest::addColumn<IntList>("expectedProxyItems");
    QTest::newRow("1")
        << (QStringList() << "a") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "" // filter
        << 0 // proxyStartRow
        << "a" // what
        << static_cast<int>(Qt::MatchExactly) // matchFlags
        << (IntList() << 0); // expectedProxyItems
    QTest::newRow("2")
        << (QStringList() << "a" << "b") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "" // filter
        << 0 // proxyStartRow
        << "b" // what
        << static_cast<int>(Qt::MatchExactly) // matchFlags
        << (IntList() << 1); // expectedProxyItems
    QTest::newRow("3")
        << (QStringList() << "a" << "b") // sourceItems
        << static_cast<int>(Qt::DescendingOrder) // sortOrder
        << "" // filter
        << 0 // proxyStartRow
        << "a" // what
        << static_cast<int>(Qt::MatchExactly) // matchFlags
        << (IntList() << 1); // expectedProxyItems
    QTest::newRow("4")
        << (QStringList() << "b" << "d" << "a" << "c") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "" // filter
        << 1 // proxyStartRow
        << "a" // what
        << static_cast<int>(Qt::MatchExactly) // matchFlags
        << IntList(); // expectedProxyItems
    QTest::newRow("5")
        << (QStringList() << "b" << "d" << "a" << "c") // sourceItems
        << static_cast<int>(Qt::AscendingOrder) // sortOrder
        << "a|b" // filter
        << 0 // proxyStartRow
        << "c" // what
        << static_cast<int>(Qt::MatchExactly) // matchFlags
        << IntList(); // expectedProxyItems
    QTest::newRow("6")
        << (QStringList() << "b" << "d" << "a" << "c") // sourceItems
        << static_cast<int>(Qt::DescendingOrder) // sortOrder
        << "a|b" // filter
        << 0 // proxyStartRow
        << "b" // what
        << static_cast<int>(Qt::MatchExactly) // matchFlags
        << (IntList() << 0); // expectedProxyItems
}

void tst_QSortFilterProxyModel::match()
{
    QFETCH(QStringList, sourceItems);
    QFETCH(int, sortOrder);
    QFETCH(QString, filter);
    QFETCH(int, proxyStartRow);
    QFETCH(QString, what);
    QFETCH(int, matchFlags);
    QFETCH(IntList, expectedProxyItems);

    QStandardItemModel model;
    QSortFilterProxyModel proxy;

    proxy.setSourceModel(&model);
    model.insertColumns(0, 1);
    model.insertRows(0, sourceItems.count());

    for (int i = 0; i < sourceItems.count(); ++i) {
        QModelIndex index = model.index(i, 0, QModelIndex());
        model.setData(index, sourceItems.at(i), Qt::DisplayRole);
    }

    proxy.sort(0, static_cast<Qt::SortOrder>(sortOrder));
    proxy.setFilterRegExp(filter);

    QModelIndex startIndex = proxy.index(proxyStartRow, 0);
    QModelIndexList indexes = proxy.match(startIndex, Qt::DisplayRole, what,
                                          expectedProxyItems.count(),
                                          static_cast<Qt::MatchFlags>(matchFlags));
    QCOMPARE(indexes.count(), expectedProxyItems.count());
    for (int i = 0; i < indexes.count(); ++i)
        QCOMPARE(indexes.at(i).row(), expectedProxyItems.at(i));
}

void tst_QSortFilterProxyModel::insertIntoChildrenlessItem()
{
    QStandardItemModel model;
    QStandardItem *itemA = new QStandardItem("a");
    model.appendRow(itemA);
    QStandardItem *itemB = new QStandardItem("b");
    model.appendRow(itemB);
    QStandardItem *itemC = new QStandardItem("c");
    model.appendRow(itemC);

    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);

    QSignalSpy colsInsertedSpy(&proxy, SIGNAL(columnsInserted(const QModelIndex&, int, int)));
    QSignalSpy rowsInsertedSpy(&proxy, SIGNAL(rowsInserted(const QModelIndex&, int, int)));

    (void)proxy.rowCount(QModelIndex()); // force mapping of "a", "b", "c"
    QCOMPARE(colsInsertedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 0);

    // now add a child to itemB ==> should get insert notification from the proxy
    itemB->appendRow(new QStandardItem("a.0"));
    QCOMPARE(colsInsertedSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 1);

    QVariantList args = colsInsertedSpy.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), proxy.mapFromSource(itemB->index()));
    QCOMPARE(qvariant_cast<int>(args.at(1)), 0);
    QCOMPARE(qvariant_cast<int>(args.at(2)), 0);

    args = rowsInsertedSpy.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), proxy.mapFromSource(itemB->index()));
    QCOMPARE(qvariant_cast<int>(args.at(1)), 0);
    QCOMPARE(qvariant_cast<int>(args.at(2)), 0);
}

void tst_QSortFilterProxyModel::invalidateMappedChildren()
{
    QStandardItemModel model;

    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);

    QStandardItem *itemA = new QStandardItem("a");
    model.appendRow(itemA);
    QStandardItem *itemB = new QStandardItem("b");
    itemA->appendRow(itemB);

    QStandardItem *itemC = new QStandardItem("c");
    itemB->appendRow(itemC);
    itemC->appendRow(new QStandardItem("d"));

    // force mappings
    (void)proxy.hasChildren(QModelIndex());
    (void)proxy.hasChildren(proxy.mapFromSource(itemA->index()));
    (void)proxy.hasChildren(proxy.mapFromSource(itemB->index()));
    (void)proxy.hasChildren(proxy.mapFromSource(itemC->index()));

    itemB->removeRow(0); // should invalidate mapping of itemC
    itemC = new QStandardItem("c");
    itemA->appendRow(itemC);
    itemC->appendRow(new QStandardItem("d"));

    itemA->removeRow(1); // should invalidate mapping of itemC
    itemC = new QStandardItem("c");
    itemB->appendRow(itemC);
    itemC->appendRow(new QStandardItem("d"));

    QCOMPARE(proxy.rowCount(proxy.mapFromSource(itemA->index())), 1);
    QCOMPARE(proxy.rowCount(proxy.mapFromSource(itemB->index())), 1);
    QCOMPARE(proxy.rowCount(proxy.mapFromSource(itemC->index())), 1);
}

class EvenOddFilterModel : public QSortFilterProxyModel
{
public:
    virtual bool filterAcceptsRow(int srcRow, const QModelIndex& srcParent) const
    {
        if (srcParent.isValid())
            return (srcParent.row() % 2) ^ !(srcRow % 2);
        return (srcRow % 2);
    }
};

void tst_QSortFilterProxyModel::insertRowIntoFilteredParent()
{
    QStandardItemModel model;
    EvenOddFilterModel proxy;
    proxy.setSourceModel(&model);

    QSignalSpy spy(&proxy, SIGNAL(rowsInserted(const QModelIndex&, int, int)));

    QStandardItem *itemA = new QStandardItem();
    model.appendRow(itemA); // A will be filtered
    QStandardItem *itemB = new QStandardItem();
    itemA->appendRow(itemB);

    QCOMPARE(spy.count(), 0);

    itemA->removeRow(0);

    QCOMPARE(spy.count(), 0);
}

void tst_QSortFilterProxyModel::filterOutParentAndFilterInChild()
{
    QStandardItemModel model;
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);

    proxy.setFilterRegExp("A|B");
    QStandardItem *itemA = new QStandardItem("A");
    model.appendRow(itemA); // not filtered
    QStandardItem *itemB = new QStandardItem("B");
    itemA->appendRow(itemB); // not filtered
    QStandardItem *itemC = new QStandardItem("C");
    itemA->appendRow(itemC); // filtered

    QSignalSpy removedSpy(&proxy, SIGNAL(rowsRemoved(const QModelIndex&, int, int)));
    QSignalSpy insertedSpy(&proxy, SIGNAL(rowsInserted(const QModelIndex&, int, int)));

    proxy.setFilterRegExp("C"); // A and B will be filtered out, C filtered in

    // we should now have been notified that the subtree represented by itemA has been removed
    QCOMPARE(removedSpy.count(), 1);
    // we should NOT get any inserts; itemC should be filtered because its parent (itemA) is
    QCOMPARE(insertedSpy.count(), 0);
}

#endif // QT_VERSION

QTEST_MAIN(tst_QSortFilterProxyModel)
#include "tst_qsortfilterproxymodel.moc"

#else // QT_VERSION
QTEST_NOOP_MAIN
#endif

