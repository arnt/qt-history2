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
#include <QtGui/QScrollBar>


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
    void moveCursor2();
    void indexAt();
    void clicked();
    void singleSelectionRemoveRow();
    void singleSelectionRemoveColumn();
    void modelColumn();
    void hideFirstRow();
    void batchedMode();
    void setCurrentIndex();
    void selection_data();
    void selection();
    void scrollTo();
    void moveItems();
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

    // make sure setViewMode() doesn't reset resizeMode
    obj1.clearPropertyFlags();
    obj1.setResizeMode(QListView::Adjust);
    obj1.setViewMode(QListView::IconMode);
    QCOMPARE(obj1.resizeMode(), QListView::Adjust);

    obj1.setWordWrap(false);
    QCOMPARE(false, obj1.wordWrap());
    obj1.setWordWrap(true);
    QCOMPARE(true, obj1. wordWrap());
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

        if (!m_icon.isNull() && role == Qt::DecorationRole) {
            return m_icon;
        }
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

    void setDataIcon(const QIcon &icon)
    {
        m_icon = icon;
    }

    int colCount, rCount;
    QIcon m_icon;
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

    // show in per-item mode, then hide the first row
    view.setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    QVERIFY(!view.isRowHidden(0));
    view.setRowHidden(0, true);
    QVERIFY(view.isRowHidden(0));
    view.setRowHidden(0, false);
    QVERIFY(!view.isRowHidden(0));

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

class QMoveCursorListView : public QListView
{
public:
    QMoveCursorListView() : QListView() {}
    
    enum CursorAction { MoveUp, MoveDown, MoveLeft, MoveRight,
        MoveHome, MoveEnd, MovePageUp, MovePageDown,
        MoveNext, MovePrevious };

    QModelIndex moveCursor(QMoveCursorListView::CursorAction action, Qt::KeyboardModifiers modifiers)
    {
        return QListView::moveCursor((QListView::CursorAction)action, modifiers);
    }
};

void tst_QListView::moveCursor2()
{
    QtTestModel model(0);
    model.colCount = 1;
    model.rCount = 100;
    QPixmap pm(32, 32);
    pm.fill(Qt::green);    
    model.setDataIcon(QIcon(pm));

    QMoveCursorListView vu;
    vu.setModel(&model);
    vu.setIconSize(QSize(36,48));
    vu.setGridSize(QSize(34,56));
    vu.resize(300,300);
    vu.setFlow(QListView::LeftToRight);
    vu.setMovement(QListView::Static);
    vu.setWrapping(true);
    vu.setViewMode(QListView::IconMode);
    vu.setLayoutMode(QListView::Batched);
    vu.show();
    vu.selectionModel()->setCurrentIndex(model.index(0,0), QItemSelectionModel::SelectCurrent);
    QCoreApplication::processEvents();

    QModelIndex idx = vu.moveCursor(QMoveCursorListView::MoveHome, Qt::NoModifier);
    QCOMPARE(idx, model.index(0,0));
    idx = vu.moveCursor(QMoveCursorListView::MoveDown, Qt::NoModifier);
    QCOMPARE(idx, model.index(8,0));

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

void tst_QListView::modelColumn()
{
    int numCols = 3;
    int numRows = 3;
    QStandardItemModel model(numCols, numRows);
    for (int r = 0; r < numRows; ++r)
        for (int c = 0; c < numCols; ++c)
            model.setData(model.index(r, c), QString("%1,%2").arg(r).arg(c));


    QListView view;
    view.setModel(&model);



    //
    // Set and get with a valid model
    //

    // Default is column 0
    QCOMPARE(view.modelColumn(), 0);

    view.setModelColumn(0);
    QCOMPARE(view.modelColumn(), 0);
    view.setModelColumn(1);
    QCOMPARE(view.modelColumn(), 1);
    view.setModelColumn(2);
    QCOMPARE(view.modelColumn(), 2);

    // Out of bound cases should not modify the modelColumn
    view.setModelColumn(-1);
    QCOMPARE(view.modelColumn(), 2);
    view.setModelColumn(INT_MAX);
    QCOMPARE(view.modelColumn(), 2);


    // See if it displays the right column using indexAt()...
    view.resize(400,400);
    view.show();

    for (int c = 0; c < 3; ++c) {
        view.setModelColumn(c);
        int startrow = 0;
        for (int y = 0; y < view.height(); ++y) {
            QModelIndex idx = view.indexAt( QPoint(1, y) );
            if (idx.row() == startrow + 1) ++startrow;
            else if (idx.row() == -1) break;
            QCOMPARE(idx.row(), startrow);
            QCOMPARE(idx.column(), c);
        }
        QCOMPARE(startrow, 2);
    }
}


void tst_QListView::hideFirstRow()
{
    QStringList items;
    for (int i=0; i <100; ++i)
        items << "item";
    QStringListModel model(items);

    QListView view;
    view.setModel(&model);
    view.setUniformItemSizes(true);
    view.setRowHidden(0,true);
    view.show();
    QTest::qWait(100);
}

void tst_QListView::batchedMode()
{
    QStringList items;
    for (int i=0; i <3; ++i)
        items << "item";
    QStringListModel model(items);

    QListView view;
    view.setModel(&model);
    view.setUniformItemSizes(true);
    view.setViewMode(QListView::ListMode);
    view.setLayoutMode(QListView::Batched);
    view.setBatchSize(2);
    view.resize(200,400);
    view.show();
    
    QTest::qWait(100);
    QBitArray ba;
    for (int y = 0; y < view.height(); ++y) {
        QModelIndex idx = view.indexAt( QPoint(1, y) );
        if (!idx.isValid())
            break;
        if (idx.row() >= ba.size())
            ba.resize(idx.row() + 1);
        ba.setBit(idx.row(), true);
    }
    QCOMPARE(ba.size(), 3);


    // Test the dynamic listview too.
    view.setViewMode(QListView::IconMode);
    view.setLayoutMode(QListView::Batched);
    view.setFlow(QListView::TopToBottom);
    view.setBatchSize(2);
    
    QTest::qWait(100);
    
    ba.clear();
    for (int y = 0; y < view.height(); ++y) {
        QModelIndex idx = view.indexAt( QPoint(1, y) );
        if (!idx.isValid())
            break;
        if (idx.row() >= ba.size())
            ba.resize(idx.row() + 1);
        ba.setBit(idx.row(), true);
    }
    QCOMPARE(ba.size(), 3);
    
}

void tst_QListView::setCurrentIndex()
{
    QStringList items;
    int i;
    for (i=0; i <20; ++i)
        items << QString("item %1").arg(i);
    QStringListModel model(items);

    QListView view;
    view.setModel(&model);

    view.resize(220,182);
    view.show();

    for (int pass = 0; pass < 2; ++pass) {
        view.setFlow(pass == 0 ? QListView::TopToBottom : QListView::LeftToRight);
        QScrollBar *sb = pass == 0 ? view.verticalScrollBar() : view.horizontalScrollBar();
        QList<QSize> gridsizes;
        gridsizes << QSize() << QSize(200,38);
        for (int ig = 0; ig < gridsizes.count(); ++ig) {
            if (pass == 1 && !gridsizes.at(ig).isValid()) // the width of an item varies, so it might jump two times
                continue;
            view.setGridSize(gridsizes.at(ig));
        
            qApp->processEvents();
            int offset = sb->value();

            // first "scroll" down, verify that we scroll one step at a time
            i = 0;
            for (i = 0; i < 20; ++i) {
                QModelIndex idx = model.index(i,0);
                view.setCurrentIndex(idx);
                if (offset != sb->value()) {
                    // If it has scrolled, it should have scrolled only by one.
                    QCOMPARE(sb->value(), offset + 1);
                    ++offset;
                }
                //QTest::qWait(50);
            }

            --i;    // item 20 does not exist
            // and then "scroll" up, verify that we scroll one step at a time
            for (; i >= 0; --i) {
                QModelIndex idx = model.index(i,0);
                view.setCurrentIndex(idx);
                if (offset != sb->value()) {
                    // If it has scrolled, it should have scrolled only by one.
                    QCOMPARE(sb->value(), offset - 1);
                    --offset;
                }
                //QTest::qWait(50);
            }
        }
    }
}

class PublicListView : public QListView
{
    public:
    PublicListView(QWidget *parent = 0) : QListView(parent)
    {

    }
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) {
        QListView::setSelection(rect, flags);
    }
    QSize contentsSize() const { return QListView::contentsSize(); }

    void setPositionForIndex(const QPoint &pos, const QModelIndex &index) {
        QListView::setPositionForIndex(pos, index);
    }
};

class TestDelegate : public QItemDelegate
{
public:
    TestDelegate(QObject *parent) : QItemDelegate(parent) {}
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const { return QSize(50, 50); }
};

typedef QList<int> IntList;
Q_DECLARE_METATYPE(IntList)

void tst_QListView::selection_data()
{
    QTest::addColumn<int>("itemCount");
    QTest::addColumn<int>("viewMode");
    QTest::addColumn<int>("flow");
    QTest::addColumn<bool>("wrapping");
    QTest::addColumn<int>("spacing");
    QTest::addColumn<QSize>("gridSize");
    QTest::addColumn<IntList>("hiddenRows");
    QTest::addColumn<QRect>("selectionRect");
    QTest::addColumn<IntList>("expectedItems");

    QTest::newRow("select all")
        << 4                                    // itemCount
        << int(QListView::ListMode)
        << int(QListView::TopToBottom)
        << false                                // wrapping
        << 0                                    // spacing
        << QSize()                              // gridSize
        << IntList()                            // hiddenRows
        << QRect(0, 0, 10, 200)                 // selection rectangle
        << (IntList() << 0 << 1 << 2 << 3);     // expected items
    
    QTest::newRow("select below, (on viewport)")
        << 4                                    // itemCount
        << int(QListView::ListMode)
        << int(QListView::TopToBottom)
        << false                                // wrapping
        << 0                                    // spacing
        << QSize()                              // gridSize
        << IntList()                            // hiddenRows
        << QRect(10, 250, 1, 1)                 // selection rectangle
        << (IntList());                         // expected items

    QTest::newRow("select below 2, (on viewport)")
        << 4                                    // itemCount
        << int(QListView::ListMode)
        << int(QListView::TopToBottom)
        << true                                 // wrapping
        << 0                                    // spacing
        << QSize()                              // gridSize
        << IntList()                            // hiddenRows
        << QRect(10, 250, 1, 1)                 // selection rectangle
        << (IntList());                         // expected items
    
    QTest::newRow("select to the right, (on viewport)")
        << 40                                   // itemCount
        << int(QListView::ListMode)
        << int(QListView::TopToBottom)
        << true                                 // wrapping
        << 0                                    // spacing
        << QSize()                              // gridSize
        << IntList()                            // hiddenRows
        << QRect(300, 10, 1, 1)                 // selection rectangle
        << (IntList());                         // expected items
    
    QTest::newRow("select to the right, (on viewport)")
        << 40                                   // itemCount
        << int(QListView::ListMode)
        << int(QListView::TopToBottom)
        << true                                 // wrapping
        << 0                                    // spacing
        << QSize()                              // gridSize
        << IntList()                            // hiddenRows
        << QRect(300, 0, 1, 300)                // selection rectangle
        << (IntList());                         // expected items

  
    QTest::newRow("select inside contents, (on viewport)")
        << 35                                   // itemCount
        << int(QListView::ListMode)
        << int(QListView::TopToBottom)
        << true                                 // wrapping
        << 0                                    // spacing
        << QSize()                              // gridSize
        << IntList()                            // hiddenRows
        << QRect(175, 275, 1, 1)                // selection rectangle
        << (IntList());                         // expected items
}

void tst_QListView::selection()
{
    QFETCH(int, itemCount);
    QFETCH(int, viewMode);
    QFETCH(int, flow);
    QFETCH(bool, wrapping);
    QFETCH(int, spacing);
    QFETCH(QSize, gridSize);
    QFETCH(IntList, hiddenRows);
    QFETCH(QRect, selectionRect);
    QFETCH(IntList, expectedItems);

    PublicListView v;
    QtTestModel model;
    model.colCount = 1;
    model.rCount = itemCount;

    v.setItemDelegate(new TestDelegate(&v));
    v.setModel(&model);
    v.setViewMode(QListView::ViewMode(viewMode));
    v.setFlow(QListView::Flow(flow));
    v.setWrapping(wrapping);
    v.setResizeMode(QListView::Adjust);
    v.setSpacing(spacing);
    if (gridSize.isValid())
        v.setGridSize(gridSize);
    for (int j = 0; j < hiddenRows.count(); ++j) {
        v.setRowHidden(hiddenRows.at(j), true);
    }
    v.resize(525,525);
    v.show();
    QApplication::processEvents();

    v.setSelection(selectionRect, QItemSelectionModel::ClearAndSelect);

    QModelIndexList selected = v.selectionModel()->selectedIndexes();
    
    QCOMPARE(selected.count(), expectedItems.count());
    for (int i = 0; i < selected.count(); ++i) {
        QVERIFY(expectedItems.contains(selected.at(i).row()));
    }

}

void tst_QListView::scrollTo()
{
    QListView lv;
    QStringListModel model(&lv);
    QStringList list;
    list << "Short item 1";
    list << "Short item 2";
    list << "Short item 3";
    list << "Short item 4";
    list << "Short item 5";
    list << "Short item 6";
    list << "Begin This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\n"
            "This is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item\nThis is a very long item End\n";
    list << "Short item";
    list << "Short item";
    list << "Short item";
    list << "Short item";
    list << "Short item";
    list << "Short item";
    list << "Short item";
    list << "Short item";
    model.setStringList(list);
    lv.setModel(&model);
    lv.setFixedSize(100, 200);
    lv.show();

    //by default, the list view scrolls per item and has no wrapping
    QModelIndex index = model.index(6,0);

    //we save the size of the item for later comparisons
    const QSize itemsize = lv.visualRect(index).size();
    QVERIFY(itemsize.height() > lv.height());
    QVERIFY(itemsize.width() > lv.width());

    //we click the item
    QPoint p = lv.visualRect(index).center();
    QTest::mouseClick(lv.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    //let's wait 1 second because the scrolling is delayed
    QTest::qWait(1000);
    QCOMPARE(lv.visualRect(index).y(),0);

    //we scroll down. As the item is to tall for the view, it will disappear
    QTest::keyClick(lv.viewport(), Qt::Key_Down, Qt::NoModifier);
    QCOMPARE(lv.visualRect(index).y(), -itemsize.height());

    QTest::keyClick(lv.viewport(), Qt::Key_Up, Qt::NoModifier);
    QCOMPARE(lv.visualRect(index).y(), 0);

    //Let's enable wrapping

    lv.setWrapping(true);
    lv.horizontalScrollBar()->setValue(0); //let's scroll to the beginning

    //we click the item
    p = lv.visualRect(index).center();
    QTest::mouseClick(lv.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    //let's wait 1 second because the scrolling is delayed
    QTest::qWait(1000);
    QCOMPARE(lv.visualRect(index).x(),0);

    //we scroll right. As the item is too wide for the view, it will disappear
    QTest::keyClick(lv.viewport(), Qt::Key_Right, Qt::NoModifier);
    QCOMPARE(lv.visualRect(index).x(), -itemsize.width());

    QTest::keyClick(lv.viewport(), Qt::Key_Left, Qt::NoModifier);
    QCOMPARE(lv.visualRect(index).x(), 0);

    lv.setWrapping(false);

    //Let's try with scrolling per pixel
    lv.setHorizontalScrollMode( QListView::ScrollPerPixel);
    lv.verticalScrollBar()->setValue(0); //scrolls back to the first item

    //we click the item
    p = lv.visualRect(index).center();
    QTest::mouseClick(lv.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    //let's wait 1 second because the scrolling is delayed
    QTest::qWait(1000);
    QCOMPARE(lv.visualRect(index).y(),0);
    
    //we scroll down. As the item is too tall for the view, it will partially disappear
    QTest::keyClick(lv.viewport(), Qt::Key_Down, Qt::NoModifier);
    QVERIFY(lv.visualRect(index).y()<0);

    QTest::keyClick(lv.viewport(), Qt::Key_Up, Qt::NoModifier);
    QCOMPARE(lv.visualRect(index).y(), 0);

}

void tst_QListView::moveItems()
{
    QStandardItemModel model;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            QStandardItem* item = new QStandardItem(QString("standard item (%1,%2)").arg(r).arg(c));
            model.setItem(r, c, item);
        }
    }

    PublicListView view;
    view.setViewMode(QListView::IconMode);
	view.setResizeMode(QListView::Fixed);
	view.setWordWrap(true);
    view.setModel(&model);
    view.setItemDelegate(new TestDelegate(&view));

	for (int r = 0; r < model.rowCount(); ++r) {
		for (int c = 0; c < model.columnCount(); ++c) {
			const QModelIndex& idx = model.index(r, c);
			view.setPositionForIndex(QPoint(r * 75, r * 75), idx);
		}
	}

    QCOMPARE(view.contentsSize(), QSize(275, 275));
}


QTEST_MAIN(tst_QListView)
#include "tst_qlistview.moc"
