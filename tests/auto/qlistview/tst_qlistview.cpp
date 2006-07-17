/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qlistview.h>
#include <qitemdelegate.h>
#include <qstandarditemmodel.h>
#include <qstringlistmodel.h>
#include <cmath>
#include <math.h>


//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qlistview.h gui/itemviews/qlistview.cpp

class tst_QListView : public QObject
{
    Q_OBJECT

public:
    tst_QListView();
    virtual ~tst_QListView();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void noDelegate();
    void noModel();
    void emptyModel();
    void removeRows();
    void cursorMove();
    void hideRows();
    void moveCursor();
    void indexAt();
    void clicked();
    void singleSelectionRemoveRow();
    void singleSelectionRemoveColumn();
};

// Testing get/set functions
void tst_QListView::getSetCheck()
{
    QListView obj1;
    // Movement QListView::movement()
    // void QListView::setMovement(Movement)
    obj1.setMovement(QListView::Movement(QListView::Static));
    QCOMPARE(QListView::Movement(QListView::Static), obj1.movement());
    obj1.setMovement(QListView::Movement(QListView::Free));
    QCOMPARE(QListView::Movement(QListView::Free), obj1.movement());
    obj1.setMovement(QListView::Movement(QListView::Snap));
    QCOMPARE(QListView::Movement(QListView::Snap), obj1.movement());

    // Flow QListView::flow()
    // void QListView::setFlow(Flow)
    obj1.setFlow(QListView::Flow(QListView::LeftToRight));
    QCOMPARE(QListView::Flow(QListView::LeftToRight), obj1.flow());
    obj1.setFlow(QListView::Flow(QListView::TopToBottom));
    QCOMPARE(QListView::Flow(QListView::TopToBottom), obj1.flow());

    // ResizeMode QListView::resizeMode()
    // void QListView::setResizeMode(ResizeMode)
    obj1.setResizeMode(QListView::ResizeMode(QListView::Fixed));
    QCOMPARE(QListView::ResizeMode(QListView::Fixed), obj1.resizeMode());
    obj1.setResizeMode(QListView::ResizeMode(QListView::Adjust));
    QCOMPARE(QListView::ResizeMode(QListView::Adjust), obj1.resizeMode());

    // LayoutMode QListView::layoutMode()
    // void QListView::setLayoutMode(LayoutMode)
    obj1.setLayoutMode(QListView::LayoutMode(QListView::SinglePass));
    QCOMPARE(QListView::LayoutMode(QListView::SinglePass), obj1.layoutMode());
    obj1.setLayoutMode(QListView::LayoutMode(QListView::Batched));
    QCOMPARE(QListView::LayoutMode(QListView::Batched), obj1.layoutMode());

    // int QListView::spacing()
    // void QListView::setSpacing(int)
    obj1.setSpacing(0);
    QCOMPARE(0, obj1.spacing());
    obj1.setSpacing(INT_MIN);
    QCOMPARE(INT_MIN, obj1.spacing());
    obj1.setSpacing(INT_MAX);
    QCOMPARE(INT_MAX, obj1.spacing());

    // ViewMode QListView::viewMode()
    // void QListView::setViewMode(ViewMode)
    obj1.setViewMode(QListView::ViewMode(QListView::ListMode));
    QCOMPARE(QListView::ViewMode(QListView::ListMode), obj1.viewMode());
    obj1.setViewMode(QListView::ViewMode(QListView::IconMode));
    QCOMPARE(QListView::ViewMode(QListView::IconMode), obj1.viewMode());

    // int QListView::modelColumn()
    // void QListView::setModelColumn(int)
    obj1.setModelColumn(0);
    QCOMPARE(0, obj1.modelColumn());
    obj1.setModelColumn(INT_MIN);
    QCOMPARE(0, obj1.modelColumn()); // Less than 0 => 0
    obj1.setModelColumn(INT_MAX);
    QCOMPARE(0, obj1.modelColumn()); // No model => 0

    // bool QListView::uniformItemSizes()
    // void QListView::setUniformItemSizes(bool)
    obj1.setUniformItemSizes(false);
    QCOMPARE(false, obj1.uniformItemSizes());
    obj1.setUniformItemSizes(true);
    QCOMPARE(true, obj1.uniformItemSizes());

#if QT_VERSION >= 0x040200
    // make sure setViewMode() doesn't reset resizeMode
    obj1.clearPropertyFlags();
    obj1.setResizeMode(QListView::Adjust);
    obj1.setViewMode(QListView::IconMode);
    QCOMPARE(obj1.resizeMode(), QListView::Adjust);
#endif
}

class QtTestModel: public QAbstractListModel
{
public:
    QtTestModel(QObject *parent = 0): QAbstractListModel(parent),
       colCount(0), rCount(0), wrongIndex(false) {}
    int rowCount(const QModelIndex&) const { return rCount; }
    int columnCount(const QModelIndex&) const { return colCount; }
    bool isEditable(const QModelIndex &) const { return true; }

    QVariant data(const QModelIndex &idx, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (idx.row() < 0 || idx.column() < 0 || idx.column() >= colCount
            || idx.row() >= rCount) {
            wrongIndex = true;
            qWarning("got invalid modelIndex %d/%d", idx.row(), idx.column());
        }
        return QString("%1/%2").arg(idx.row()).arg(idx.column());
    }

    void removeLastRow()
    {
        beginRemoveRows(QModelIndex(), rCount - 2, rCount - 1);
        --rCount;
        endRemoveRows();
    }

    void removeAllRows()
    {
        beginRemoveRows(QModelIndex(), 0, rCount - 1);
        rCount = 0;
        endRemoveRows();
    }

    int colCount, rCount;
    mutable bool wrongIndex;
};

tst_QListView::tst_QListView()
{
}

tst_QListView::~tst_QListView()
{
}

void tst_QListView::initTestCase()
{
}

void tst_QListView::cleanupTestCase()
{
}

void tst_QListView::init()
{
}

void tst_QListView::cleanup()
{
}


void tst_QListView::noDelegate()
{
    QtTestModel model(0);
    model.rCount = model.colCount = 10;
    QListView view;
    view.setModel(&model);
    view.setItemDelegate(0);
    view.show();
}

void tst_QListView::noModel()
{
    QListView view;
    view.show();
    view.setRowHidden(0, true);
}

void tst_QListView::emptyModel()
{
    QtTestModel model(0);
    QListView view;
    view.setModel(&model);
    view.show();
    QVERIFY(!model.wrongIndex);
}

void tst_QListView::removeRows()
{
    QtTestModel model(0);
    model.rCount = model.colCount = 10;

    QListView view;
    view.setModel(&model);
    view.show();

    model.removeLastRow();
    QVERIFY(!model.wrongIndex);

    model.removeAllRows();
    QVERIFY(!model.wrongIndex);
}

void tst_QListView::cursorMove()
{
    int rows = 6*6;
    int columns = 6;

    QStandardItemModel model(rows, columns);
    QListView view;
    view.setModel(&model);

    for (int j = 0; j < columns; ++j) {
        view.setModelColumn(j);
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = model.index(i, j);
            model.setData(index, QString("[%1,%2]").arg(i).arg(j));
            view.setCurrentIndex(index);
            QApplication::processEvents();
            QCOMPARE(view.currentIndex(), index);
        }
    }

    QSize cellsize(60, 25);
    int gap = 1; // compensate for the scrollbars
    int displayColumns = 6;

    view.resize((displayColumns + gap) * cellsize.width(),
                 int((ceil(double(rows) / displayColumns) + gap) * cellsize.height()));
    view.setResizeMode(QListView::Adjust);
    view.setGridSize(cellsize);
    view.setViewMode(QListView::IconMode);
    view.doItemsLayout();
    view.show();

    QVector<Qt::Key> keymoves;
    keymoves << Qt::Key_Up << Qt::Key_Up << Qt::Key_Right << Qt::Key_Right << Qt::Key_Up
             << Qt::Key_Left << Qt::Key_Left << Qt::Key_Up << Qt::Key_Down << Qt::Key_Up
             << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up
             << Qt::Key_Left << Qt::Key_Left << Qt::Key_Up << Qt::Key_Down;

    int displayRow    = rows / displayColumns - 1;
    int displayColumn = displayColumns - (rows % displayColumns) - 1;

    QApplication::instance()->processEvents();
    for (int i = 0; i < keymoves.size(); ++i) {
        Qt::Key key = keymoves.at(i);
        QTest::keyClick(&view, key);
        switch (key) {
        case Qt::Key_Up:
            displayRow = qMax(0, displayRow - 1);
            break;
        case Qt::Key_Down:
            displayRow = qMin(rows / displayColumns - 1, displayRow + 1);
            break;
        case Qt::Key_Left:
            displayColumn = qMax(0, displayColumn - 1);
            break;
        case Qt::Key_Right:
            displayColumn = qMin(displayColumns-1, displayColumn + 1);
            break;
        default:
            QVERIFY(false);
        }

        QApplication::instance()->processEvents();

        int row = displayRow * displayColumns + displayColumn;
        int column = columns - 1;
        QModelIndex index = model.index(row, column);
        QCOMPARE(view.currentIndex().row(), row);
        QCOMPARE(view.currentIndex().column(), column);
        QCOMPARE(view.currentIndex(), index);
    }
}

void tst_QListView::hideRows()
{
    QtTestModel model(0);
    model.rCount = model.colCount = 10;

    QListView view;
    view.setModel(&model);
    view.show();

    // hide then show
    QVERIFY(!view.isRowHidden(2));
    view.setRowHidden(2, true);
    QVERIFY(view.isRowHidden(2));
    view.setRowHidden(2, false);
    QVERIFY(!view.isRowHidden(2));

    // re show same row
    QVERIFY(!view.isRowHidden(2));
    view.setRowHidden(2, false);
    QVERIFY(!view.isRowHidden(2));

    // double hidding
    QVERIFY(!view.isRowHidden(2));
    view.setRowHidden(2, true);
    QVERIFY(view.isRowHidden(2));
    view.setRowHidden(2, true);
    QVERIFY(view.isRowHidden(2));
    view.setRowHidden(2, false);
    QVERIFY(!view.isRowHidden(2));

}

void tst_QListView::moveCursor()
{
    QtTestModel model(0);
    model.rCount = model.colCount = 10;

    QListView view;
    view.setModel(&model);

    QTest::keyClick(&view, Qt::Key_Down);

    view.setModel(0);
    view.setModel(&model);
    view.setRowHidden(0, true);

    QTest::keyClick(&view, Qt::Key_Down);
    QCOMPARE(view.selectionModel()->currentIndex(), model.index(1, 0));
}


class QListViewShowEventListener : public QListView
{
public:
    QListViewShowEventListener() : QListView() { m_shown = false;}

    virtual void showEvent(QShowEvent * /*e*/)
    {
        int columnwidth = sizeHintForColumn(0);
        QSize sz = sizeHintForIndex(model()->index(0,0));

        // This should retrieve a model index in the 2nd section
        m_index = indexAt(QPoint(columnwidth +2, sz.height()/2));
        m_shown = true;
    }

    QModelIndex m_index;
    bool m_shown;

};

void tst_QListView::indexAt()
{
    QtTestModel model(0);
    model.rCount = 2;
    model.colCount = 1;

    QListView view;
    view.setModel(&model);
    view.setViewMode(QListView::ListMode);
    view.setFlow(QListView::TopToBottom);

    QSize sz = view.sizeHintForIndex(model.index(0,0));
    QModelIndex index;
    index = view.indexAt(QPoint(20,0));
    QVERIFY(index.isValid());
    QCOMPARE(index.row(), 0);

    index = view.indexAt(QPoint(20,sz.height()));
    QVERIFY(index.isValid());
    QCOMPARE(index.row(), 1);

    index = view.indexAt(QPoint(20,2 * sz.height()));
    QVERIFY(!index.isValid());

    

    model.rCount = 30;
    QListViewShowEventListener view2;
    // Set the height to a small enough value so that it wraps to a new section.
    view2.resize(300,100);
    view2.setModel(&model);
    view2.setFlow(QListView::TopToBottom);
    view2.setViewMode(QListView::ListMode);
    view2.setWrapping(true);
    // We really want to make sure it is shown, because the layout won't be known until it is shown
    view2.show();
    for (int i = 0; i < 5 && !view2.m_shown; ++i) {
        QTest::qWait(500);
    }
    
    QVERIFY(view2.m_index.isValid());
    QVERIFY(view2.m_index.row() != 0);

}

void tst_QListView::clicked()
{
    QtTestModel model;
    model.rCount = 10;
    model.colCount = 2;

    qRegisterMetaType<QModelIndex>("QModelIndex");

    QListView view;
    view.setModel(&model);

    view.show();
    QApplication::processEvents();

    QModelIndex firstIndex = model.index(0, 0, QModelIndex());
    QVERIFY(firstIndex.isValid());
    int itemHeight = view.visualRect(firstIndex).height();
    view.resize(200, itemHeight * (model.rCount + 1));

    for (int i = 0; i < model.rCount; ++i) {
        QPoint p(5, 1 + itemHeight * i);
        QModelIndex index = view.indexAt(p);
        if (!index.isValid())
            continue;
        QSignalSpy spy(&view, SIGNAL(clicked(const QModelIndex&)));
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
        QCOMPARE(spy.count(), 1);
    }
}

void tst_QListView::singleSelectionRemoveRow()
{
    QStringList items;
    items << "item1" << "item2" << "item3" << "item4";
    QStringListModel model(items);

    QListView view;
    view.setModel(&model);
    view.show();

    QModelIndex index;
    view.setCurrentIndex(model.index(1));
    index = view.currentIndex();
    QCOMPARE(view.model()->data(index).toString(), QString("item2"));

    model.removeRow(1);
    index = view.currentIndex();
    QCOMPARE(view.model()->data(index).toString(), QString("item1"));

    model.removeRow(0);
    index = view.currentIndex();
    QCOMPARE(view.model()->data(index).toString(), QString("item3"));
}

void tst_QListView::singleSelectionRemoveColumn()
{
    int numCols = 3;
    int numRows = 3;
    QStandardItemModel model(numCols, numRows);
    for (int r = 0; r < numRows; ++r)
        for (int c = 0; c < numCols; ++c)
            model.setData(model.index(r, c), QString("%1,%2").arg(r).arg(c));

    QListView view;
    view.setModel(&model);
    view.show();

    QModelIndex index;
    view.setCurrentIndex(model.index(1, 1));
    index = view.currentIndex();
    QCOMPARE(view.model()->data(index).toString(), QString("1,1"));

    model.removeColumn(1);
    index = view.currentIndex();
    QCOMPARE(view.model()->data(index).toString(), QString("1,0"));

    model.removeColumn(0);
    index = view.currentIndex();
    QCOMPARE(view.model()->data(index).toString(), QString("1,2"));
}

QTEST_MAIN(tst_QListView)
#include "tst_qlistview.moc"
