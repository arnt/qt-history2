/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QStandardItemModel>

#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qheaderview.h>
#include <qitemdelegate.h>
#include <qtreewidget.h>
#include <qdebug.h>

typedef QList<int> IntList;
Q_DECLARE_METATYPE(IntList)

typedef QList<bool> BoolList;
Q_DECLARE_METATYPE(BoolList)

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qheaderview.h gui/itemviews/qheaderview.cpp

// Will try to wait for the condition while allowing event processing
// for a maximum of 2 seconds.
#define WAIT_FOR_CONDITION(expr, expected) \
    do { \
        const int step = 100; \
        for (int i = 0; i < 2000 && expr != expected; i+=step) { \
            QTest::qWait(step); \
        } \
    } while(0)

class protected_QHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    protected_QHeaderView(Qt::Orientation orientation) : QHeaderView(orientation) {
        resizeSections();
    };

    void testEvent();
    void testhorizontalOffset();
    void testverticalOffset();
};

class tst_QHeaderView : public QObject
{
    Q_OBJECT

public:
    tst_QHeaderView();
    virtual ~tst_QHeaderView();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void getSetCheck();
    void visualIndex();

    void visualIndexAt_data();
    void visualIndexAt();

    void noModel();
    void emptyModel();
    void removeRows();
    void removeCols();

    void clickable();
    void movable();
    void hidden();
    void stretch();

    void sectionSize_data();
    void sectionSize();

    void length();
    void offset();
    void sectionSizeHint();
    void logicalIndex();
    void logicalIndexAt();
    void swapSections();

    void moveSection_data();
    void moveSection();

    void resizeMode();

    void resizeSection_data();
    void resizeSection();

    void resizeAndMoveSection_data();
    void resizeAndMoveSection();
    void resizeHiddenSection_data();
    void resizeHiddenSection();
    void resizeAndInsertSection_data();
    void resizeAndInsertSection();
    void moveAndInsertSection_data();
    void moveAndInsertSection();
    void highlightSections();
    void showSortIndicator();
    void removeAndInsertRow();
    void unhideSection();
    void event();
    void headerDataChanged();
    void currentChanged();
    void horizontalOffset();
    void verticalOffset();
    void stretchSectionCount();
    void hiddenSectionCount();
    void focusPolicy();
    void moveSectionAndReset();
    void moveSectionAndRemove();
    void saveRestore();

    void defaultAlignment_data();
    void defaultAlignment();

    void globalResizeMode_data();
    void globalResizeMode();

protected:
    QHeaderView *view;
    QStandardItemModel *model;
};

class QtTestModel: public QAbstractTableModel
{

Q_OBJECT

public:
    QtTestModel(QObject *parent = 0): QAbstractTableModel(parent),
       cols(0), rows(0), wrongIndex(false) {}
    int rowCount(const QModelIndex&) const { return rows; }
    int columnCount(const QModelIndex&) const { return cols; }
    bool isEditable(const QModelIndex &) const { return true; }

    QVariant data(const QModelIndex &idx, int) const
    {
        if (idx.row() < 0 || idx.column() < 0 || idx.column() >= cols || idx.row() >= rows) {
            wrongIndex = true;
            qWarning("Invalid modelIndex [%d,%d,%p]", idx.row(), idx.column(), idx.internalPointer());
        }
        return QString("[%1,%2,%3]").arg(idx.row()).arg(idx.column()).arg(0);//idx.data());
    }

    void removeLastRow()
    {
        beginRemoveRows(QModelIndex(), rows - 1, rows - 1);
        --rows;
        endRemoveRows();
    }

    void removeAllRows()
    {
        beginRemoveRows(QModelIndex(), 0, rows - 1);
        rows = 0;
        endRemoveRows();
    }

    void removeLastColumn()
    {
        beginRemoveColumns(QModelIndex(), cols - 1, cols - 1);
        --cols;
        endRemoveColumns();
    }

    void removeAllColumns()
    {
        beginRemoveColumns(QModelIndex(), 0, cols - 1);
        cols = 0;
        endRemoveColumns();
    }

    void cleanup()
    {
        cols = 3;
        rows = 3;
        emit layoutChanged();
    }

    int cols, rows;
    mutable bool wrongIndex;
};

// Testing get/set functions
void tst_QHeaderView::getSetCheck()
{
    protected_QHeaderView obj1(Qt::Horizontal);
    // bool QHeaderView::highlightSections()
    // void QHeaderView::setHighlightSections(bool)
    obj1.setHighlightSections(false);
    QCOMPARE(false, obj1.highlightSections());
    obj1.setHighlightSections(true);
    QCOMPARE(true, obj1.highlightSections());

    // bool QHeaderView::stretchLastSection()
    // void QHeaderView::setStretchLastSection(bool)
    obj1.setStretchLastSection(false);
    QCOMPARE(false, obj1.stretchLastSection());
    obj1.setStretchLastSection(true);
    QCOMPARE(true, obj1.stretchLastSection());

    // int QHeaderView::defaultSectionSize()
    // void QHeaderView::setDefaultSectionSize(int)
    obj1.setDefaultSectionSize(0);
    QCOMPARE(0, obj1.defaultSectionSize());
    obj1.setDefaultSectionSize(INT_MIN);
    QCOMPARE(INT_MIN, obj1.defaultSectionSize());
    obj1.setDefaultSectionSize(INT_MAX);
    QCOMPARE(INT_MAX, obj1.defaultSectionSize());
    // ### the test above does not make sense for values below 0

#if QT_VERSION >= 0x040200
    // int QHeaderView::minimumSectionSize()
    // void QHeaderView::setMinimumSectionSize(int)
    obj1.setMinimumSectionSize(0);
    QCOMPARE(0, obj1.minimumSectionSize());
    obj1.setMinimumSectionSize(INT_MIN);
    QCOMPARE(INT_MIN, obj1.minimumSectionSize());
    obj1.setMinimumSectionSize(INT_MAX);
    QCOMPARE(INT_MAX, obj1.minimumSectionSize());
    // ### the test above does not make sense for values below 0
#endif

    // int QHeaderView::offset()
    // void QHeaderView::setOffset(int)
    obj1.setOffset(0);
    QCOMPARE(0, obj1.offset());
    obj1.setOffset(INT_MIN);
    QCOMPARE(INT_MIN, obj1.offset());
    obj1.setOffset(INT_MAX);
    QCOMPARE(INT_MAX, obj1.offset());

}

tst_QHeaderView::tst_QHeaderView()
{
}

tst_QHeaderView::~tst_QHeaderView()
{
}

void tst_QHeaderView::initTestCase()
{
}

void tst_QHeaderView::cleanupTestCase()
{
}

void tst_QHeaderView::init()
{
    view = new QHeaderView(Qt::Vertical);
    // Some initial value tests before a model is added
    QCOMPARE(view->length(), 0);
    QVERIFY(view->sizeHint() == QSize(0,0));
    QCOMPARE(view->sectionSizeHint(0), -1);

    /*
    model = new QStandardItemModel(1, 1);
    view->setModel(model);
    //qDebug() << view->count();
    view->sizeHint();
    */

    int rows = 4;
    int columns = 4;
    model = new QStandardItemModel(rows, columns);
    /*
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            QModelIndex index = model->index(row, column, QModelIndex());
            model->setData(index, QVariant((row+1) * (column+1)));
        }
    }
    */

    QSignalSpy spy(view, SIGNAL(sectionCountChanged(int, int)));
    view->setModel(model);
    QCOMPARE(spy.count(), 1);
    view->show();
}

void tst_QHeaderView::cleanup()
{
    delete view;
    view = 0;
    delete model;
    model = 0;
}

void tst_QHeaderView::noModel()
{
    QHeaderView emptyView(Qt::Vertical);
    QCOMPARE(emptyView.count(), 0);
}

void tst_QHeaderView::emptyModel()
{
    QtTestModel testmodel;
    view->setModel(&testmodel);
    QVERIFY(!testmodel.wrongIndex);
    QCOMPARE(view->count(), testmodel.rows);
    view->setModel(model);
}

void tst_QHeaderView::removeRows()
{
    QtTestModel model;
    model.rows = model.cols = 10;

    QHeaderView vertical(Qt::Vertical);
    QHeaderView horizontal(Qt::Horizontal);

    vertical.setModel(&model);
    horizontal.setModel(&model);
    vertical.show();
    horizontal.show();
    QCOMPARE(vertical.count(), model.rows);
    QCOMPARE(horizontal.count(), model.cols);

    model.removeLastRow();
    QVERIFY(!model.wrongIndex);
    QCOMPARE(vertical.count(), model.rows);
    QCOMPARE(horizontal.count(), model.cols);

    model.removeAllRows();
    QVERIFY(!model.wrongIndex);
    QCOMPARE(vertical.count(), model.rows);
    QCOMPARE(horizontal.count(), model.cols);
}


void tst_QHeaderView::removeCols()
{
    QtTestModel model;
    model.rows = model.cols = 10;

    QHeaderView vertical(Qt::Vertical);
    QHeaderView horizontal(Qt::Horizontal);
    vertical.setModel(&model);
    horizontal.setModel(&model);
    vertical.show();
    horizontal.show();
    QCOMPARE(vertical.count(), model.rows);
    QCOMPARE(horizontal.count(), model.cols);

    model.removeLastColumn();
    QVERIFY(!model.wrongIndex);
    QCOMPARE(vertical.count(), model.rows);
    QCOMPARE(horizontal.count(), model.cols);

    model.removeAllColumns();
    QVERIFY(!model.wrongIndex);
    QCOMPARE(vertical.count(), model.rows);
    QCOMPARE(horizontal.count(), model.cols);
}

void tst_QHeaderView::movable()
{
    QCOMPARE(view->isMovable(), false);
    view->setMovable(false);
    QCOMPARE(view->isMovable(), false);
    view->setMovable(true);
    QCOMPARE(view->isMovable(), true);
}

void tst_QHeaderView::clickable()
{
    QCOMPARE(view->isClickable(), false);
    view->setClickable(false);
    QCOMPARE(view->isClickable(), false);
    view->setClickable(true);
    QCOMPARE(view->isClickable(), true);
}

void tst_QHeaderView::hidden()
{
    //hideSection() & showSection call setSectionHidden
    // Test bad arguments
    QCOMPARE(view->isSectionHidden(-1), false);
    QCOMPARE(view->isSectionHidden(view->count()), false);
    QCOMPARE(view->isSectionHidden(999999), false);

    view->setSectionHidden(-1, true);
    view->setSectionHidden(view->count(), true);
    view->setSectionHidden(999999, true);
    view->setSectionHidden(-1, false);
    view->setSectionHidden(view->count(), false);
    view->setSectionHidden(999999, false);

    // Hidden sections shouldn't have visual properties (except position)
    int pos = view->defaultSectionSize();
    view->setSectionHidden(1, true);
    QCOMPARE(view->sectionSize(1), 0);
    QCOMPARE(view->sectionPosition(1), pos);
    view->resizeSection(1, 100);
    QCOMPARE(view->sectionViewportPosition(1), pos);
    QCOMPARE(view->sectionSize(1), 0);
    view->setSectionHidden(1, false);
    QCOMPARE(view->isSectionHidden(0), false);
    QCOMPARE(view->sectionSize(0), view->defaultSectionSize());
}

void tst_QHeaderView::stretch()
{
    // Show before resize and setStrechLastSection
    view->resize(500, 500);
    view->setStretchLastSection(true);
    QCOMPARE(view->stretchLastSection(), true);
    view->show();
    QCOMPARE(view->width(), 500);
    QCOMPARE(view->visualIndexAt(view->viewport()->height() - 5), 3);

    view->setSectionHidden(3, true);
    QCOMPARE(view->visualIndexAt(view->viewport()->height() - 5), 2);

    view->setStretchLastSection(false);
    QCOMPARE(view->stretchLastSection(), false);
}

void tst_QHeaderView::sectionSize_data()
{
    QTest::addColumn<QList<int> >("boundsCheck");
    QTest::addColumn<QList<int> >("defaultSizes");
    QTest::addColumn<int>("initialDefaultSize");
    QTest::addColumn<int>("lastVisibleSectionSize");
    QTest::addColumn<int>("persistentSectionSize");

    QTest::newRow("data set one")
        << (QList<int>() << -1 << 0 << 4 << 9999)
        << (QList<int>() << 10 << 30 << 30)
        << 30
        << 300
        << 20;
}

void tst_QHeaderView::sectionSize()
{
    QFETCH(QList<int>, boundsCheck);
    QFETCH(QList<int>, defaultSizes);
    QFETCH(int, initialDefaultSize);
    QFETCH(int, lastVisibleSectionSize);
    QFETCH(int, persistentSectionSize);

    // bounds check
    foreach (int val, boundsCheck)
        view->sectionSize(val);

    // default size
    QCOMPARE(view->defaultSectionSize(), initialDefaultSize);
    foreach (int def, defaultSizes) {
        view->setDefaultSectionSize(def);
        QCOMPARE(view->defaultSectionSize(), def);
    }

    view->setDefaultSectionSize(initialDefaultSize);
    for (int s = 0; s < view->count(); ++s)
        QCOMPARE(view->sectionSize(s), initialDefaultSize);

    // stretch last section
    view->setStretchLastSection(true);
    int lastSection = view->count() - 1;
    #if 0 // ### disabled
    // even with stretchLastSection,  the stretched last section can't become smaller than the minimum section size.
    if (view->length() > view->viewport()->height()) {
        int minimumSize = view->fontMetrics().height() + view->style()->pixelMetric(QStyle::PM_HeaderMargin);
        QCOMPARE(view->sectionSize(lastSection), minimumSize);
    } else {
        QCOMPARE(view->length(), view->viewport()->height());
        int minimumSize = view->viewport()->height() - view->sectionViewportPosition(lastSection);
        QCOMPARE(view->sectionSize(lastSection), minimumSize);
    }
    #endif

    
    //test that when hiding the last column, 
    //resizing the new last visible columns still works
    view->hideSection(lastSection);
    view->resizeSection(lastSection - 1, lastVisibleSectionSize);
    QCOMPARE(view->sectionSize(lastSection - 1), lastVisibleSectionSize);
    view->showSection(lastSection);

    // turn off stretching
    view->setStretchLastSection(false);
    QCOMPARE(view->sectionSize(lastSection), initialDefaultSize);

    // test persistence
    int sectionCount = view->count();
    for (int i = 0; i < sectionCount; ++i)
        view->resizeSection(i, persistentSectionSize);
    QtTestModel model;
    model.cols = sectionCount * 2;
    model.rows = sectionCount * 2;
    view->setModel(&model);
    for (int j = 0; j < sectionCount; ++j)
        QCOMPARE(view->sectionSize(j), persistentSectionSize);
    for (int k = sectionCount; k < view->count(); ++k)
        QCOMPARE(view->sectionSize(k), initialDefaultSize);
}

void tst_QHeaderView::visualIndex()
{
    // Test bad arguments
    QCOMPARE(view->visualIndex(999999), -1);
    QCOMPARE(view->visualIndex(-1), -1);
    QCOMPARE(view->visualIndex(1), 1);
    view->setSectionHidden(1, true);
    QCOMPARE(view->visualIndex(1), 1);
    QCOMPARE(view->visualIndex(2), 2);

    view->setSectionHidden(1, false);
    QCOMPARE(view->visualIndex(1), 1);
    QCOMPARE(view->visualIndex(2), 2);
}

void tst_QHeaderView::visualIndexAt_data()
{
    QTest::addColumn<QList<int> >("hidden");
    QTest::addColumn<QList<int> >("from");
    QTest::addColumn<QList<int> >("to");
    QTest::addColumn<QList<int> >("coordinate");
    QTest::addColumn<QList<int> >("visual");

    QTest::newRow("no hidden, no moved sections")
        << QList<int>()
        << QList<int>()
        << QList<int>()
        << (QList<int>() << -1 << 0 << 31 << 91 << 99999)
        << (QList<int>() << -1 << 0 << 1 << 3 << -1);

    QTest::newRow("no hidden, moved sections")
        << QList<int>()
        << (QList<int>() << 0)
        << (QList<int>() << 1)
        << (QList<int>() << -1 << 0 << 31 << 91 << 99999)
        << (QList<int>() << -1 << 0 << 1 << 3 << -1);

    QTest::newRow("hidden, no moved sections")
        << (QList<int>() << 0)
        << QList<int>()
        << QList<int>()
        << (QList<int>() << -1 << 0 << 31 << 91 << 99999)
        << (QList<int>() << -1 << 1 << 2 << 3 << -1);
}

void tst_QHeaderView::visualIndexAt()
{
    QFETCH(QList<int>, hidden);
    QFETCH(QList<int>, from);
    QFETCH(QList<int>, to);
    QFETCH(QList<int>, coordinate);
    QFETCH(QList<int>, visual);

    view->setStretchLastSection(true);
    view->show();

    for (int i = 0; i < hidden.count(); ++i)
        view->setSectionHidden(hidden.at(i), true);

    for (int j = 0; j < from.count(); ++j)
        view->moveSection(from.at(j), to.at(j));

    for (int k = 0; k < coordinate.count(); ++k)
        QCOMPARE(view->visualIndexAt(coordinate.at(k)), visual.at(k));
}

void tst_QHeaderView::length()
{
    view->setStretchLastSection(true);
    view->show();

    //minimumSectionSize should be the size of the last section of the widget is not tall enough
    int length = view->minimumSectionSize();
    for (int i=0; i < view->count()-1; i++) {
        length += view->sectionSize(i);
    }

    length = qMax(length, view->viewport()->height());
    QCOMPARE(length, view->length());

    view->setStretchLastSection(false);
    view->show();

    QVERIFY(length != view->length());

    // layoutChanged might mean rows have been removed
    QtTestModel model;
    model.cols = 10;
    model.rows = 10;
    view->setModel(&model);
    int oldLength = view->length();
    model.cleanup();
    QCOMPARE(model.rows, view->count());
    QVERIFY(oldLength != view->length());
}

void tst_QHeaderView::offset()
{
    QCOMPARE(view->offset(), 0);
    view->setOffset(10);
    QCOMPARE(view->offset(), 10);
    view->setOffset(0);
    QCOMPARE(view->offset(), 0);

    // Test odd arguments
    view->setOffset(-1);
}

void tst_QHeaderView::sectionSizeHint()
{
    // Test bad arguments
    view->sectionSizeHint(-1);
    view->sectionSizeHint(99999);

    // TODO how to test the return value?
}

void tst_QHeaderView::logicalIndex()
{
    // Test bad arguments
    QCOMPARE(view->logicalIndex(-1), -1);
    QCOMPARE(view->logicalIndex(99999), -1);
}

void tst_QHeaderView::logicalIndexAt()
{
    // Test bad arguments
    view->logicalIndexAt(-1);
    view->logicalIndexAt(99999);
    QCOMPARE(view->logicalIndexAt(0), 0);
    QCOMPARE(view->logicalIndexAt(1), 0);

    view->show();
    view->setStretchLastSection(true);
    // First item
    QCOMPARE(view->logicalIndexAt(0), 0);
    QCOMPARE(view->logicalIndexAt(view->sectionSize(0)-1), 0);
    QCOMPARE(view->logicalIndexAt(view->sectionSize(0)+1), 1);
    // Last item
    int last = view->length() - 1;//view->viewport()->height() - 10;
    QCOMPARE(view->logicalIndexAt(last), 3);
    // Not in widget
    int outofbounds = view->length() + 1;//view->viewport()->height() + 1;
    QCOMPARE(view->logicalIndexAt(outofbounds), -1);

    view->moveSection(0,1);
    // First item
    QCOMPARE(view->logicalIndexAt(0), 1);
    QCOMPARE(view->logicalIndexAt(view->sectionSize(0)-1), 1);
    QCOMPARE(view->logicalIndexAt(view->sectionSize(0)+1), 0);
    // Last item
    QCOMPARE(view->logicalIndexAt(last), 3);
    view->moveSection(1,0);

}

void tst_QHeaderView::swapSections()
{
    view->swapSections(-1, 1);
    view->swapSections(99999, 1);
    view->swapSections(1, -1);
    view->swapSections(1, 99999);

    QVector<int> logical = (QVector<int>() << 0 << 1 << 2 << 3);

    QSignalSpy spy1(view, SIGNAL(sectionMoved(int, int, int)));

    QCOMPARE(view->sectionsMoved(), false);
    view->swapSections(1, 1);
    QCOMPARE(view->sectionsMoved(), false);
    view->swapSections(1, 2);
    QCOMPARE(view->sectionsMoved(), true);
    view->swapSections(2, 1);
    QCOMPARE(view->sectionsMoved(), true);
    for (int i = 0; i < view->count(); ++i)
        QCOMPARE(view->logicalIndex(i), logical.at(i));
    QCOMPARE(spy1.count(), 4);

    logical = (QVector<int>()  << 3 << 1 << 2 << 0);
    view->swapSections(3, 0);
    QCOMPARE(view->sectionsMoved(), true);
    for (int j = 0; j < view->count(); ++j)
        QCOMPARE(view->logicalIndex(j), logical.at(j));
    QCOMPARE(spy1.count(), 6);
}

void tst_QHeaderView::moveSection_data()
{
    QTest::addColumn<QList<int> >("hidden");
    QTest::addColumn<QList<int> >("from");
    QTest::addColumn<QList<int> >("to");
    QTest::addColumn<QList<bool> >("moved");
    QTest::addColumn<QList<int> >("logical");
    QTest::addColumn<int>("count");

    QTest::newRow("bad args, no hidden")
        << QList<int>()
        << (QList<int>() << -1 << 1 << 99999 << 1)
        << (QList<int>() << 1 << -1 << 1 << 99999)
        << (QList<bool>() << false << false << false << false)
        << (QList<int>() << 0 << 1 << 2 << 3)
        << 0;

    QTest::newRow("good args, no hidden")
        << QList<int>()
        << (QList<int>() << 1 << 1 << 2 << 1)
        << (QList<int>() << 1 << 2 << 1 << 2)
        << (QList<bool>() << false << true << true << true)
        << (QList<int>() << 0 << 2 << 1 << 3)
        << 3;

    QTest::newRow("hidden sections")
        << (QList<int>() << 0 << 3)
        << (QList<int>() << 1 << 1 << 2 << 1)
        << (QList<int>() << 1 << 2 << 1 << 2)
        << (QList<bool>() << false << true << true << true)
        << (QList<int>() << 0 << 2 << 1 << 3)
        << 3;
}

void tst_QHeaderView::moveSection()
{
    QFETCH(QList<int>, hidden);
    QFETCH(QList<int>, from);
    QFETCH(QList<int>, to);
    QFETCH(QList<bool>, moved);
    QFETCH(QList<int>, logical);
    QFETCH(int, count);

    QVERIFY(from.count() == to.count());
    QVERIFY(from.count() == moved.count());
    QVERIFY(view->count() == logical.count());

    QSignalSpy spy1(view, SIGNAL(sectionMoved(int, int, int)));
    QCOMPARE(view->sectionsMoved(), false);

    for (int h = 0; h < hidden.count(); ++h)
        view->setSectionHidden(hidden.at(h), true);

    for (int i = 0; i < from.count(); ++i) {
        view->moveSection(from.at(i), to.at(i));
        QCOMPARE(view->sectionsMoved(), moved.at(i));
    }

    for (int j = 0; j < view->count(); ++j)
        QCOMPARE(view->logicalIndex(j), logical.at(j));

    QCOMPARE(spy1.count(), count);
}

void tst_QHeaderView::resizeAndMoveSection_data()
{
    QTest::addColumn<IntList>("logicalIndexes");
    QTest::addColumn<IntList>("sizes");
    QTest::addColumn<int>("logicalFrom");
    QTest::addColumn<int>("logicalTo");

    QTest::newRow("resizeAndMove-1")
        << (IntList() << 0 << 1)
        << (IntList() << 20 << 40)
        << 0 << 1;

    QTest::newRow("resizeAndMove-2")
        << (IntList() << 0 << 1 << 2 << 3)
        << (IntList() << 20 << 60 << 10 << 80)
        << 0 << 2;

    QTest::newRow("resizeAndMove-3")
        << (IntList() << 0 << 1 << 2 << 3)
        << (IntList() << 100 << 60 << 40 << 10)
        << 0 << 3;

    QTest::newRow("resizeAndMove-4")
        << (IntList() << 0 << 1 << 2 << 3)
        << (IntList() << 10 << 40 << 80 << 30)
        << 1 << 2;

    QTest::newRow("resizeAndMove-5")
        << (IntList() << 2 << 3)
        << (IntList() << 100 << 200)
        << 3 << 2;
}

void tst_QHeaderView::resizeAndMoveSection()
{
    QFETCH(IntList, logicalIndexes);
    QFETCH(IntList, sizes);
    QFETCH(int, logicalFrom);
    QFETCH(int, logicalTo);

    // Save old visual indexes and sizes
    IntList oldVisualIndexes;
    IntList oldSizes;
    foreach (int logical, logicalIndexes) {
        oldVisualIndexes.append(view->visualIndex(logical));
        oldSizes.append(view->sectionSize(logical));
    }

    // Resize sections
    for (int i = 0; i < logicalIndexes.size(); ++i) {
        int logical = logicalIndexes.at(i);
        view->resizeSection(logical, sizes.at(i));
    }

    // Move sections
    int visualFrom = view->visualIndex(logicalFrom);
    int visualTo = view->visualIndex(logicalTo);
    view->moveSection(visualFrom, visualTo);
    QCOMPARE(view->visualIndex(logicalFrom), visualTo);

    // Check that sizes are still correct
    for (int i = 0; i < logicalIndexes.size(); ++i) {
        int logical = logicalIndexes.at(i);
        QCOMPARE(view->sectionSize(logical), sizes.at(i));
    }

    // Move sections back
    view->moveSection(visualTo, visualFrom);

    // Check that sizes are still correct
    for (int i = 0; i < logicalIndexes.size(); ++i) {
        int logical = logicalIndexes.at(i);
        QCOMPARE(view->sectionSize(logical), sizes.at(i));
    }

    // Put everything back as it was
    for (int i = 0; i < logicalIndexes.size(); ++i) {
        int logical = logicalIndexes.at(i);
        view->resizeSection(logical, oldSizes.at(i));
        QCOMPARE(view->visualIndex(logical), oldVisualIndexes.at(i));
    }
}

void tst_QHeaderView::resizeHiddenSection_data()
{
    QTest::addColumn<int>("section");
    QTest::addColumn<int>("initialSize");
    QTest::addColumn<int>("finalSize");

    QTest::newRow("section 0 resize 50 to 20")
        << 0 << 50 << 20;

    QTest::newRow("section 1 resize 50 to 20")
        << 1 << 50 << 20;

    QTest::newRow("section 2 resize 50 to 20")
        << 2 << 50 << 20;

    QTest::newRow("section 3 resize 50 to 20")
        << 3 << 50 << 20;
}

void tst_QHeaderView::resizeHiddenSection()
{
    QFETCH(int, section);
    QFETCH(int, initialSize);
    QFETCH(int, finalSize);

    view->resizeSection(section, initialSize);
    view->setSectionHidden(section, true);
    QCOMPARE(view->sectionSize(section), 0);

    view->resizeSection(section, finalSize);
    QCOMPARE(view->sectionSize(section), 0);

    view->setSectionHidden(section, false);
    QCOMPARE(view->sectionSize(section), finalSize);
}

void tst_QHeaderView::resizeAndInsertSection_data()
{
    QTest::addColumn<int>("section");
    QTest::addColumn<int>("size");
    QTest::addColumn<int>("insert");
    QTest::addColumn<int>("compare");
    QTest::addColumn<int>("expected");

    QTest::newRow("section 0 size 50 insert 0")
        << 0 << 50 << 0 << 1 << 50;

    QTest::newRow("section 1 size 50 insert 1")
        << 0 << 50 << 1 << 0 << 50;

    QTest::newRow("section 1 size 50 insert 0")
        << 1 << 50 << 0 << 2 << 50;

}

void tst_QHeaderView::resizeAndInsertSection()
{
    QFETCH(int, section);
    QFETCH(int, size);
    QFETCH(int, insert);
    QFETCH(int, compare);
    QFETCH(int, expected);

    view->setStretchLastSection(false);
    
    view->resizeSection(section, size);
    QCOMPARE(view->sectionSize(section), size);

    model->insertRow(insert);
    
    QCOMPARE(view->sectionSize(compare), expected);
}

void tst_QHeaderView::moveAndInsertSection_data()
{
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("to");
    QTest::addColumn<int>("insert");
    QTest::addColumn<QList<int> >("mapping");

    QTest::newRow("move from 1 to 3, insert 0")
        << 1 << 3 << 0 <<(QList<int>() << 0 << 1 << 3 << 4 << 2);

}

void tst_QHeaderView::moveAndInsertSection()
{
    QFETCH(int, from);
    QFETCH(int, to);
    QFETCH(int, insert);
    QFETCH(QList<int>, mapping);

    view->setStretchLastSection(false);
    
    view->moveSection(from, to);

    model->insertRow(insert);

    for (int i = 0; i < mapping.count(); ++i)
        QCOMPARE(view->logicalIndex(i), mapping.at(i));
}

void tst_QHeaderView::resizeMode()
{
    // Q_ASSERT's when resizeMode is called with an invalid index
    int last = view->count() - 1;
    view->setResizeMode(QHeaderView::Interactive);
    QCOMPARE(view->resizeMode(last), QHeaderView::Interactive);
    QCOMPARE(view->resizeMode(1), QHeaderView::Interactive);
    view->setResizeMode(QHeaderView::Stretch);
    QCOMPARE(view->resizeMode(last), QHeaderView::Stretch);
    QCOMPARE(view->resizeMode(1), QHeaderView::Stretch);
    view->setResizeMode(QHeaderView::Custom);
    QCOMPARE(view->resizeMode(last), QHeaderView::Custom);
    QCOMPARE(view->resizeMode(1), QHeaderView::Custom);

    // test when sections have been moved
    view->setStretchLastSection(false);
    for (int i=0; i < (view->count() - 1); ++i)
        view->setResizeMode(i, QHeaderView::Interactive);
    int logicalIndex = view->count() / 2;
    view->setResizeMode(logicalIndex, QHeaderView::Stretch);
    view->moveSection(view->visualIndex(logicalIndex), 0);
    for (int i=0; i < (view->count() - 1); ++i) {
        if (i == logicalIndex)
            QCOMPARE(view->resizeMode(i), QHeaderView::Stretch);
        else
            QCOMPARE(view->resizeMode(i), QHeaderView::Interactive);
    }
}

void tst_QHeaderView::resizeSection_data()
{
    QTest::addColumn<int>("initial");
    QTest::addColumn<QList<int> >("logical");
    QTest::addColumn<QList<int> >("size");
    QTest::addColumn<QList<int> >("mode");
    QTest::addColumn<int>("resized");
    QTest::addColumn<QList<int> >("expected");

    QTest::newRow("bad args")
        << 100
        << (QList<int>() << -1 << -1 << 99999 << 99999 << 4)
        << (QList<int>() << -1 << 0 << 99999 << -1 << -1)
        << (QList<int>()
            << int(QHeaderView::Interactive)
            << int(QHeaderView::Interactive)
            << int(QHeaderView::Interactive)
            << int(QHeaderView::Interactive))
        << 0
        << (QList<int>() << 0 << 0 << 0 << 0 << 0);
}

void tst_QHeaderView::resizeSection()
{
    
    QFETCH(int, initial);
    QFETCH(QList<int>, logical);
    QFETCH(QList<int>, size);
    QFETCH(QList<int>, mode);
    QFETCH(int, resized);
    QFETCH(QList<int>, expected);

    view->resize(400, 400);

    view->show();
    view->setMovable(true);
    view->setStretchLastSection(false);

    for (int i = 0; i < logical.count(); ++i)
        if (logical.at(i) > -1 && logical.at(i) < view->count()) // for now
            view->setResizeMode(logical.at(i), (QHeaderView::ResizeMode)mode.at(i));

    for (int j = 0; j < logical.count(); ++j)
        view->resizeSection(logical.at(j), initial);

    QSignalSpy spy(view, SIGNAL(sectionResized(int, int, int)));

    for (int k = 0; k < logical.count(); ++k)
        view->resizeSection(logical.at(k), size.at(k));

    QCOMPARE(spy.count(), resized);

    for (int l = 0; l < logical.count(); ++l)
        QCOMPARE(view->sectionSize(logical.at(l)), expected.at(l));
}

void tst_QHeaderView::highlightSections()
{
    view->setHighlightSections(true);
    QCOMPARE(view->highlightSections(), true);
    view->setHighlightSections(false);
    QCOMPARE(view->highlightSections(), false);
}

void tst_QHeaderView::showSortIndicator()
{
    view->setSortIndicatorShown(true);
    QCOMPARE(view->isSortIndicatorShown(), true);
    QCOMPARE(view->sortIndicatorOrder(), Qt::DescendingOrder);
    view->setSortIndicator(1, Qt::AscendingOrder);
    QCOMPARE(view->sortIndicatorOrder(), Qt::AscendingOrder);
    view->setSortIndicator(1, Qt::DescendingOrder);
    QCOMPARE(view->sortIndicatorOrder(), Qt::DescendingOrder);
    view->setSortIndicatorShown(false);
    QCOMPARE(view->isSortIndicatorShown(), false);

    view->setSortIndicator(999999, Qt::DescendingOrder);
    // Don't segfult baby :)
    view->setSortIndicatorShown(true);
}


void tst_QHeaderView::removeAndInsertRow()
{
    // Check if logicalIndex returns the correct value after we have removed a row
    // we might as well te
    for (int i = 0; i < model->rowCount(); ++i) {
        QCOMPARE(i, view->logicalIndex(i));
    }

    while (model->removeRow(0)) {
        for (int i = 0; i < model->rowCount(); ++i) {
            QCOMPARE(i, view->logicalIndex(i));
        }
    }

    int pass = 0;
    for (pass = 0; pass < 5; pass++) {
        for (int i = 0; i < model->rowCount(); ++i) {
            QCOMPARE(i, view->logicalIndex(i));
        }
        model->insertRow(0);
    }

    while (model->removeRows(0, 2)) {
        for (int i = 0; i < model->rowCount(); ++i) {
            QCOMPARE(i, view->logicalIndex(i));
        }
    }

    for (pass = 0; pass < 3; pass++) {
        model->insertRows(0, 2);
        for (int i = 0; i < model->rowCount(); ++i) {
            QCOMPARE(i, view->logicalIndex(i));
        }
    }

    for (pass = 0; pass < 3; pass++) {
        model->insertRows(3, 2);
        for (int i = 0; i < model->rowCount(); ++i) {
            QCOMPARE(i, view->logicalIndex(i));
        }
    }

    // Insert at end
    for (pass = 0; pass < 3; pass++) {
        int rowCount = model->rowCount();
        model->insertRows(rowCount, 1);
        for (int i = 0; i < rowCount; ++i) {
            QCOMPARE(i, view->logicalIndex(i));
        }
    }

}
void tst_QHeaderView::unhideSection()
{
    // You should not necessarily expect the same size back again, so the best test we can do is to test if it is larger than 0 after a unhide.
    QCOMPARE(view->sectionsHidden(), false);
    view->setSectionHidden(0, true);
    QCOMPARE(view->sectionsHidden(), true);
    QVERIFY(view->sectionSize(0) == 0);
    view->setResizeMode(QHeaderView::Interactive);
    view->setSectionHidden(0, false);
    QVERIFY(view->sectionSize(0) > 0);

    view->setSectionHidden(0, true);
    QVERIFY(view->sectionSize(0) == 0);
    view->setSectionHidden(0, true);
    QVERIFY(view->sectionSize(0) == 0);
    view->setResizeMode(QHeaderView::Stretch);
    view->setSectionHidden(0, false);
    QVERIFY(view->sectionSize(0) > 0);

}

void tst_QHeaderView::event()
{
    protected_QHeaderView x(Qt::Vertical);
    x.testEvent();
    protected_QHeaderView y(Qt::Horizontal);
    y.testEvent();
}


void protected_QHeaderView::testEvent()
{
    // No crashy please
    QHoverEvent enterEvent(QEvent::HoverEnter, QPoint(), QPoint());
    event(&enterEvent);
    QHoverEvent eventLeave(QEvent::HoverLeave, QPoint(), QPoint());
    event(&eventLeave);
    QHoverEvent eventMove(QEvent::HoverMove, QPoint(), QPoint());
    event(&eventMove);
}

void tst_QHeaderView::headerDataChanged()
{
    // This shouldn't asserver because view is Vertical
    view->headerDataChanged(Qt::Horizontal, -1, -1);
#if 0
    // This will assert
    view->headerDataChanged(Qt::Vertical, -1, -1);
#endif

    // No crashing please
    view->headerDataChanged(Qt::Horizontal, 0, 1);
    view->headerDataChanged(Qt::Vertical, 0, 1);
}

void tst_QHeaderView::currentChanged()
{
    view->setCurrentIndex(QModelIndex());
}

void tst_QHeaderView::horizontalOffset()
{
    protected_QHeaderView x(Qt::Vertical);
    x.testhorizontalOffset();
    protected_QHeaderView y(Qt::Horizontal);
    y.testhorizontalOffset();
}

void tst_QHeaderView::verticalOffset()
{
    protected_QHeaderView x(Qt::Vertical);
    x.testverticalOffset();
    protected_QHeaderView y(Qt::Horizontal);
    y.testverticalOffset();
}

void  protected_QHeaderView::testhorizontalOffset()
{
    if(orientation() == Qt::Horizontal){
        QCOMPARE(horizontalOffset(), 0);
        setOffset(10);
        QCOMPARE(horizontalOffset(), 10);
    }
    else
        QCOMPARE(horizontalOffset(), 0);

}

void  protected_QHeaderView::testverticalOffset()
{
    if(orientation() == Qt::Vertical){
        QCOMPARE(verticalOffset(), 0);
        setOffset(10);
        QCOMPARE(verticalOffset(), 10);
    }
    else
        QCOMPARE(verticalOffset(), 0);
}

void tst_QHeaderView::stretchSectionCount()
{
    view->setStretchLastSection(false);
    QCOMPARE(view->stretchSectionCount(), 0);
    view->setStretchLastSection(true);
    QCOMPARE(view->stretchSectionCount(), 0);

    view->setResizeMode(0, QHeaderView::Stretch);
    QCOMPARE(view->stretchSectionCount(), 1);
}

void tst_QHeaderView::hiddenSectionCount()
{
    model->clear();
    model->insertRows(0, 10);
    // Hide every other one
    for (int i=0; i<10; i++)
        view->setSectionHidden(i, (i & 1) == 0);

    QCOMPARE(view->hiddenSectionCount(), 5);

    // Regression bug that occured before 4.1 release
    view->setResizeMode(QHeaderView::Stretch);
    QCOMPARE(view->hiddenSectionCount(), 5);

    // Remove some rows and make sure they are now still counted
    model->removeRow(9);
    model->removeRow(8);
    model->removeRow(7);
    model->removeRow(6);
    QCOMPARE(view->count(), 6);
    QCOMPARE(view->hiddenSectionCount(), 3);
    model->removeRows(0,5);
    QCOMPARE(view->count(), 1);
    QCOMPARE(view->hiddenSectionCount(), 0);
    QVERIFY(view->count() >=  view->hiddenSectionCount());
}

void tst_QHeaderView::focusPolicy()
{
    QHeaderView view(Qt::Horizontal);
    QCOMPARE(view.focusPolicy(), Qt::NoFocus);

    QTreeWidget widget;
    QCOMPARE(widget.header()->focusPolicy(), Qt::NoFocus);
    QVERIFY(!widget.focusProxy());
    QVERIFY(!widget.hasFocus());
    QVERIFY(!widget.header()->focusProxy());
    QVERIFY(!widget.header()->hasFocus());

    widget.show();
    widget.setFocus(Qt::OtherFocusReason);

    qApp->processEvents();

    WAIT_FOR_CONDITION(widget.hasFocus(), true);

    QVERIFY(widget.hasFocus());
    QVERIFY(!widget.header()->hasFocus());

    widget.setFocusPolicy(Qt::NoFocus);
    widget.clearFocus();

    qApp->processEvents();
    qApp->processEvents();
    
    QVERIFY(!widget.hasFocus());
    QVERIFY(!widget.header()->hasFocus());

    QTest::keyPress(&widget, Qt::Key_Tab);

    qApp->processEvents();
    qApp->processEvents();

    QVERIFY(!widget.hasFocus());
    QVERIFY(!widget.header()->hasFocus());
}

class SimpleModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    SimpleModel( QObject* parent=0)
        : QAbstractItemModel(parent),
        m_col_count(3) {}

    QModelIndex parent(const QModelIndex &/*child*/) const
    {
        return QModelIndex();
    }
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const
    {
        return hasIndex(row, column, parent) ? createIndex(row, column, 0) : QModelIndex();
    }
    int rowCount(const QModelIndex & /* parent */) const
    {
        return 8;
    }
    int columnCount(const QModelIndex &/*parent= QModelIndex()*/) const
    {
        return m_col_count;
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
        {
            return QVariant();
        }
        if (role == Qt::DisplayRole) {
            return QString::fromAscii("%1,%2").arg(index.row()).arg(index.column());
        }
        return QVariant();
    }

    void setColumnCount( int c )
    {
        m_col_count = c;
    }

private:
    int m_col_count;
};

void tst_QHeaderView::moveSectionAndReset()
{
    SimpleModel m;
    QHeaderView v(Qt::Horizontal);
    v.setModel(&m);
    int cc = 2;
    for (cc = 2; cc < 4; ++cc) {
        m.setColumnCount(cc);
        int movefrom = 0;
        int moveto;
        for (moveto = 1; moveto < cc; ++moveto) {
            v.moveSection(movefrom, moveto);
            m.setColumnCount(cc - 1);
            v.reset();
            for (int i = 0; i < cc - 1; ++i) {
                QCOMPARE(v.logicalIndex(v.visualIndex(i)), i);
            }
        }
    }
}

void tst_QHeaderView::moveSectionAndRemove()
{
    QStandardItemModel m;
    QHeaderView v(Qt::Horizontal);

    v.setModel(&m);
    v.model()->insertColumns(0, 3);
    v.moveSection(0, 1);

    QCOMPARE(v.count(), 3);
    v.model()->removeColumns(0, v.model()->columnCount());
    QCOMPARE(v.count(), 0);
}

void tst_QHeaderView::saveRestore()
{
    SimpleModel m;
    QHeaderView h1(Qt::Horizontal);
    h1.setModel(&m);
    h1.swapSections(0, 2);
    h1.resizeSection(1, 10);
    QByteArray s1 = h1.saveState();

    QHeaderView h2(Qt::Vertical);
    h2.setModel(&m);
    h2.restoreState(s1);

    QCOMPARE(h2.logicalIndex(0), 2);
    QCOMPARE(h2.logicalIndex(2), 0);
    QCOMPARE(h2.sectionSize(1), 10);

    QByteArray s2 = h2.saveState();

    QVERIFY(s1 == s2);
}

void tst_QHeaderView::defaultAlignment_data()
{
    QTest::addColumn<int>("direction");
    QTest::addColumn<int>("initial");
    QTest::addColumn<int>("alignment");

    QTest::newRow("horizontal right aligned")
        << int(Qt::Horizontal)
        << int(Qt::AlignCenter)
        << int(Qt::AlignRight);

    QTest::newRow("horizontal left aligned")
        << int(Qt::Horizontal)
        << int(Qt::AlignCenter)
        << int(Qt::AlignLeft);

    QTest::newRow("vertical right aligned")
        << int(Qt::Vertical)
        << int(Qt::AlignLeft|Qt::AlignVCenter)
        << int(Qt::AlignRight);

    QTest::newRow("vertical left aligned")
        << int(Qt::Vertical)
        << int(Qt::AlignLeft|Qt::AlignVCenter)
        << int(Qt::AlignLeft);
}

void tst_QHeaderView::defaultAlignment()
{
    QFETCH(int, direction);
    QFETCH(int, initial);
    QFETCH(int, alignment);

    SimpleModel m;

    QHeaderView header((Qt::Orientation)direction);
    header.setModel(&m);

    QCOMPARE(header.defaultAlignment(), (Qt::Alignment)initial);
    header.setDefaultAlignment((Qt::Alignment)alignment);
    QCOMPARE(header.defaultAlignment(), (Qt::Alignment)alignment);
}


void tst_QHeaderView::globalResizeMode_data()
{
    QTest::addColumn<int>("direction");
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("insert");

    QTest::newRow("horizontal ResizeToContents 0")
        << int(Qt::Horizontal)
        << int(QHeaderView::ResizeToContents)
        << 0;
}

void tst_QHeaderView::globalResizeMode()
{
    QFETCH(int, direction);
    QFETCH(int, mode);
    QFETCH(int, insert);
    
    QStandardItemModel m(4, 4);
    QHeaderView h((Qt::Orientation)direction);
    h.setModel(&m);

    h.setResizeMode((QHeaderView::ResizeMode)mode);
    m.insertRow(insert);
    for (int i = 0; i < h.count(); ++i)
        QCOMPARE(h.resizeMode(i), (QHeaderView::ResizeMode)mode);
}
QTEST_MAIN(tst_QHeaderView)
#include "tst_qheaderview.moc"
