/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/QtGui>
#include <QtTest/QtTest>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qtableview.h gui/itemviews/qtableview.cpp

// Will try to wait for the condition while allowing event processing
// for a maximum of 2 seconds.
#define WAIT_FOR_CONDITION(expr, expected) \
    do { \
        const int step = 100; \
        for (int i = 0; i < 2000 && expr != expected; i+=step) { \
            QTest::qWait(step); \
        } \
    } while(0)

typedef QList<int> IntList;
Q_DECLARE_METATYPE(IntList)

typedef QList<bool> BoolList;
Q_DECLARE_METATYPE(BoolList)

class tst_QTableView : public QObject
{
    Q_OBJECT

public:
    tst_QTableView();
    virtual ~tst_QTableView();

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

    void removeRows_data();
    void removeRows();

    void removeColumns_data();
    void removeColumns();

    void keyboardNavigation_data();
    void keyboardNavigation();

    void headerSections_data();
    void headerSections();

    void moveCursor_data();
    void moveCursor();

    void hideRows_data();
    void hideRows();

    void hideColumns_data();
    void hideColumns();

    void selection_data();
    void selection();

    void selectRow_data();
    void selectRow();

    void selectColumn_data();
    void selectColumn();

    void visualRect_data();
    void visualRect();

    void fetchMore();
    void setHeaders();

    void resizeRowsToContents_data();
    void resizeRowsToContents();

    void resizeColumnsToContents_data();
    void resizeColumnsToContents();

    void rowViewportPosition_data();
    void rowViewportPosition();

    void rowAt_data();
    void rowAt();

    void rowHeight_data();
    void rowHeight();

    void columnViewportPosition_data();
    void columnViewportPosition();

    void columnAt_data();
    void columnAt();

    void columnWidth_data();
    void columnWidth();

    void hiddenRow_data();
    void hiddenRow();

    void hiddenColumn_data();
    void hiddenColumn();

    void sortingEnabled_data();
    void sortingEnabled();

    void scrollTo_data();
    void scrollTo();

    void indexAt_data();
    void indexAt();

    void span_data();
    void span();

    void checkHeaderReset();
    void checkHeaderMinSize();

    void tabFocus();
};

// Testing get/set functions
void tst_QTableView::getSetCheck()
{
    QTableView obj1;

    obj1.setSortingEnabled(false);
    QCOMPARE(false, obj1.isSortingEnabled());
    obj1.setSortingEnabled(true);
    QCOMPARE(true, obj1.isSortingEnabled());

    obj1.setShowGrid(false);
    QCOMPARE(false, obj1.showGrid());
    obj1.setShowGrid(true);
    QCOMPARE(true, obj1.showGrid());

    obj1.setGridStyle(Qt::NoPen);
    QCOMPARE(Qt::NoPen, obj1.gridStyle());
    obj1.setGridStyle(Qt::SolidLine);
    QCOMPARE(Qt::SolidLine, obj1.gridStyle());

    obj1.setRootIndex(QModelIndex());
    QCOMPARE(QModelIndex(), obj1.rootIndex());
    QStandardItemModel model(10, 10);
    obj1.setModel(&model);
    QModelIndex index = model.index(0, 0);
    obj1.setRootIndex(index);
    QCOMPARE(index, obj1.rootIndex());

    QHeaderView *var1 = new QHeaderView(Qt::Horizontal);
    obj1.setHorizontalHeader(var1);
    QCOMPARE(var1, obj1.horizontalHeader());
    obj1.setHorizontalHeader((QHeaderView *)0);
    QCOMPARE(var1, obj1.horizontalHeader());
    delete var1;

    QHeaderView *var2 = new QHeaderView(Qt::Vertical);
    obj1.setVerticalHeader(var2);
    QCOMPARE(var2, obj1.verticalHeader());
    obj1.setVerticalHeader((QHeaderView *)0);
    QCOMPARE(var2, obj1.verticalHeader());
    delete var2;

    QCOMPARE(obj1.isCornerButtonEnabled(), true);
    obj1.setCornerButtonEnabled(false);
    QCOMPARE(obj1.isCornerButtonEnabled(), false);
}

class QtTestTableModel: public QAbstractTableModel
{
    Q_OBJECT

signals:
    void invalidIndexEncountered() const;

public:
    QtTestTableModel(int rows = 0, int columns = 0, QObject *parent = 0)
        : QAbstractTableModel(parent),
          row_count(rows),
          column_count(columns),
          can_fetch_more(false),
          fetch_more_count(0) {}

    int rowCount(const QModelIndex& = QModelIndex()) const { return row_count; }
    int columnCount(const QModelIndex& = QModelIndex()) const { return column_count; }
    bool isEditable(const QModelIndex &) const { return true; }

    QVariant data(const QModelIndex &idx, int role) const
    {
        if (!idx.isValid() || idx.row() >= row_count || idx.column() >= column_count) {
            qWarning() << "Invalid modelIndex [%d,%d,%p]" << idx;
            emit invalidIndexEncountered();
            return QVariant();
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole)
            return QString("[%1,%2,%3]").arg(idx.row()).arg(idx.column()).arg(0);

        return QVariant();
    }

    void removeLastRow()
    {
        beginRemoveRows(QModelIndex(), row_count - 1, row_count - 1);
        --row_count;
        endRemoveRows();
    }

    void removeAllRows()
    {
        beginRemoveRows(QModelIndex(), 0, row_count - 1);
        row_count = 0;
        endRemoveRows();
    }

    void removeLastColumn()
    {
        beginRemoveColumns(QModelIndex(), column_count - 1, column_count - 1);
        --column_count;
        endRemoveColumns();
    }

    void removeAllColumns()
    {
        beginRemoveColumns(QModelIndex(), 0, column_count - 1);
        column_count = 0;
        endRemoveColumns();
    }

    bool canFetchMore(const QModelIndex &) const
    {
        return can_fetch_more;
    }

    void fetchMore(const QModelIndex &)
    {
        ++fetch_more_count;
    }

    void reset()
    {
        QAbstractTableModel::reset();
    }

    int row_count;
    int column_count;
    bool can_fetch_more;
    int fetch_more_count;
};

class QtTestTableView : public QTableView
{
public:
    enum CursorAction {
        MoveUp       = QAbstractItemView::MoveUp,
        MoveDown     = QAbstractItemView::MoveDown,
        MoveLeft     = QAbstractItemView::MoveLeft,
        MoveRight    = QAbstractItemView::MoveRight,
        MoveHome     = QAbstractItemView::MoveHome,
        MoveEnd      = QAbstractItemView::MoveEnd,
        MovePageUp   = QAbstractItemView::MovePageUp,
        MovePageDown = QAbstractItemView::MovePageDown,
        MoveNext     = QAbstractItemView::MoveNext,
        MovePrevious = QAbstractItemView::MovePrevious
    };

    QModelIndex moveCursor(QtTestTableView::CursorAction cursorAction,
                           Qt::KeyboardModifiers modifiers)
    {
        return QTableView::moveCursor((QAbstractItemView::CursorAction)cursorAction, modifiers);
    }

    int columnWidthHint(int column) const
    {
        return sizeHintForColumn(column);
    }

    int rowHeightHint(int row) const
    {
        return sizeHintForRow(row);
    }

    bool isIndexHidden(const QModelIndex &index) const
    {
        return QTableView::isIndexHidden(index);
    }

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
    {
        QTableView::setSelection(rect, command);
    }

    QModelIndexList selectedIndexes() const
    {
        return QTableView::selectedIndexes();
    }
};

class QtTestItemDelegate : public QItemDelegate
{
public:
    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
    {
        return hint;
    }

    QSize hint;
};

tst_QTableView::tst_QTableView()
{
}

tst_QTableView::~tst_QTableView()
{
}

void tst_QTableView::initTestCase()
{
}

void tst_QTableView::cleanupTestCase()
{
}

void tst_QTableView::init()
{
}

void tst_QTableView::cleanup()
{
}

void tst_QTableView::noDelegate()
{
    QtTestTableModel model(10, 10);
    QTableView view;
    view.setModel(&model);
    view.setItemDelegate(0);
    view.show();
}

void tst_QTableView::noModel()
{
    QTableView view;
    view.show();
}

void tst_QTableView::emptyModel()
{
    QtTestTableModel model;
    QTableView view;
    QSignalSpy spy(&model, SIGNAL(invalidIndexEncountered()));
    view.setModel(&model);
    view.show();
    QCOMPARE(spy.count(), 0);
}

void tst_QTableView::removeRows_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");

    QTest::newRow("2x2") << 2 << 2;
    QTest::newRow("10x10") << 10  << 10;
}

void tst_QTableView::removeRows()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);

    QtTestTableModel model(rowCount, columnCount);
    QSignalSpy spy(&model, SIGNAL(invalidIndexEncountered()));

    QTableView view;
    view.setModel(&model);
    view.show();

    model.removeLastRow();
    QCOMPARE(spy.count(), 0);

    model.removeAllRows();
    QCOMPARE(spy.count(), 0);
}

void tst_QTableView::removeColumns_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");

    QTest::newRow("2x2") << 2 << 2;
    QTest::newRow("10x10") << 10  << 10;
}

void tst_QTableView::removeColumns()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);

    QtTestTableModel model(rowCount, columnCount);
    QSignalSpy spy(&model, SIGNAL(invalidIndexEncountered()));

    QTableView view;
    view.setModel(&model);
    view.show();

    model.removeLastColumn();
    QCOMPARE(spy.count(), 0);

    model.removeAllColumns();
    QCOMPARE(spy.count(), 0);
}

void tst_QTableView::keyboardNavigation_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<IntList>("keyPresses");

    QTest::newRow("16x16 model") << 16  << 16
                                 << (IntList()
                                     << Qt::Key_Up
                                     << Qt::Key_Up
                                     << Qt::Key_Right
                                     << Qt::Key_Right
                                     << Qt::Key_Up
                                     << Qt::Key_Left
                                     << Qt::Key_Left
                                     << Qt::Key_Up
                                     << Qt::Key_Down
                                     << Qt::Key_Up
                                     << Qt::Key_Up
                                     << Qt::Key_Up
                                     << Qt::Key_Up
                                     << Qt::Key_Up
                                     << Qt::Key_Up
                                     << Qt::Key_Left
                                     << Qt::Key_Left
                                     << Qt::Key_Up
                                     << Qt::Key_Down);
}

void tst_QTableView::keyboardNavigation()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(IntList, keyPresses);

    QtTestTableModel model(rowCount, columnCount);
    QTableView view;
    view.setModel(&model);

    view.show();

    QModelIndex index = model.index(rowCount - 1, columnCount - 1);
    view.setCurrentIndex(index);

    QApplication::instance()->processEvents();

    int row = rowCount - 1;
    int column = columnCount - 1;
    for (int i = 0; i < keyPresses.count(); ++i) {

        Qt::Key key = (Qt::Key)keyPresses.at(i);

        switch (key) {
        case Qt::Key_Up:
            row = qMax(0, row - 1);
            break;
        case Qt::Key_Down:
            row = qMin(rowCount - 1, row + 1);
            break;
        case Qt::Key_Left:
            column = qMax(0, column - 1);
            break;
        case Qt::Key_Right:
            column = qMin(columnCount - 1, column + 1);
            break;
        default:
            break;
        }

        QTest::keyClick(&view, key);
        QApplication::instance()->processEvents();

        QModelIndex index = model.index(row, column);
        QCOMPARE(view.currentIndex(), index);
    }
}

void tst_QTableView::headerSections_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("rowHeight");
    QTest::addColumn<int>("columnWidth");

    QTest::newRow("") << 10 << 10 << 5 << 5 << 30 << 30;
}

void tst_QTableView::headerSections()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(int, rowHeight);
    QFETCH(int, columnWidth);

    QtTestTableModel model(rowCount, columnCount);

    QTableView view;
    QHeaderView *hheader = view.horizontalHeader();
    QHeaderView *vheader = view.verticalHeader();

    view.setModel(&model);
    view.show();

    hheader->doItemsLayout();
    vheader->doItemsLayout();

    QCOMPARE(hheader->count(), model.columnCount());
    QCOMPARE(vheader->count(), model.rowCount());

    view.setRowHeight(row, rowHeight);
    QCOMPARE(view.rowHeight(row), rowHeight);
    view.hideRow(row);
    QCOMPARE(view.rowHeight(row), 0);
    view.showRow(row);
    QCOMPARE(view.rowHeight(row), rowHeight);

    view.setColumnWidth(column, columnWidth);
    QCOMPARE(view.columnWidth(column), columnWidth);
    view.hideColumn(column);
    QCOMPARE(view.columnWidth(column), 0);
    view.showColumn(column);
    QCOMPARE(view.columnWidth(column), columnWidth);
}

void tst_QTableView::moveCursor_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("hideRow");
    QTest::addColumn<int>("hideColumn");

    QTest::addColumn<int>("startRow");
    QTest::addColumn<int>("startColumn");

    QTest::addColumn<int>("cursorMoveAction");
    QTest::addColumn<int>("modifier");

    QTest::addColumn<int>("expectedRow");
    QTest::addColumn<int>("expectedColumn");

    // MoveRight
    QTest::newRow("MoveRight (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << 0 << 1;

    QTest::newRow("MoveRight (3,0)")
        << 4 << 4 << -1 << -1
        << 3 << 0
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << 3 << 1;

    QTest::newRow("MoveRight (3,3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << 3 << 3; // ###

    QTest::newRow("MoveRight, hidden column 1 (0,0)")
        << 4 << 4 << -1 << 1
        << 0 << 0
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << 0 << 2;

    QTest::newRow("MoveRight, hidden column 3 (0,2)")
        << 4 << 4 << -1 << 3
        << 0 << 2
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << 0 << 2; // ###

    // MoveNext should in addition wrap
    QTest::newRow("MoveNext (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 1;

    QTest::newRow("MoveNext (0,2)")
        << 4 << 4 << -1 << -1
        << 0 << 2
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 3;

    QTest::newRow("MoveNext, wrap (0,3)")
        << 4 << 4 << -1 << -1
        << 0 << 3
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 1 << 0;

    QTest::newRow("MoveNext, wrap (3,3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveNext, hidden column 1 (0,0)")
        << 4 << 4 << -1 << 1
        << 0 << 0
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 2;

    QTest::newRow("MoveNext, wrap, hidden column 3 (0,2)")
        << 4 << 4 << -1 << 3
        << 0 << 2
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 1 << 0;

    QTest::newRow("MoveNext, wrap, hidden column 3 (3,2)")
        << 4 << 4 << -1 << 3
        << 3 << 2
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveNext, wrapy, wrapx, hidden column 3, hidden row 3 (2,2)")
        << 4 << 4 << 3 << 3
        << 2 << 2
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 0;

    // MoveLeft
    QTest::newRow("MoveLeft (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveLeft (0,3)")
        << 4 << 4 << -1 << -1
        << 0 << 3
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << 0 << 2;

    QTest::newRow("MoveLeft (1,0)")
        << 4 << 4 << -1 << -1
        << 1 << 0
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << 1 << 0;

    QTest::newRow("MoveLeft, hidden column 0 (0,2)")
        << 4 << 4 << -1 << 1
        << 0 << 2
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveLeft, hidden column 0 (0,1)")
        << 4 << 4 << -1 << 0
        << 0 << 1
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << 0 << 1;

    // MovePrevious should in addition wrap
    QTest::newRow("MovePrevious (0,3)")
        << 4 << 4 << -1 << -1
        << 0 << 3
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 2;

    QTest::newRow("MovePrevious (0,1)")
        << 4 << 4 << -1 << -1
        << 0 << 1
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MovePrevious, wrap (1,0)")
        << 4 << 4 << -1 << -1
        << 1 << 0
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 3;

    QTest::newRow("MovePrevious, wrap, (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 3 << 3;

    QTest::newRow("MovePrevious, hidden column 1 (0,2)")
        << 4 << 4 << -1 << 1
        << 0 << 2
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MovePrevious, wrap, hidden column 3 (0,2)")
        << 4 << 4 << -1 << 3
        << 0 << 2
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 1;

    QTest::newRow("MovePrevious, wrapy, hidden column 0 (0,1)")
        << 4 << 4 << -1 << 0
        << 0 << 1
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 3 << 3;

    QTest::newRow("MovePrevious, wrap, hidden column 0, hidden row 0 (1,1)")
        << 4 << 4 << 0 << 0
        << 1 << 1
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 3 << 3;

    // MoveDown
    QTest::newRow("MoveDown (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << 1 << 0;

    QTest::newRow("MoveDown (3,0)")
        << 4 << 4 << -1 << -1
        << 3 << 0
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << 3 << 0;

    QTest::newRow("MoveDown (3,3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << 3 << 3;

    QTest::newRow("MoveDown, hidden row 1 (0,0)")
        << 4 << 4 << 1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << 2 << 0;

    QTest::newRow("MoveDown, hidden row 3 (2,0)")
        << 4 << 4 << 3 << -1
        << 2 << 0
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << 2 << 0;

    QTest::newRow("MoveDown, hidden row 0 hidden column 0 (0,0)")
        << 4 << 4 << 0 << 0
        << 0 << 0
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << 1 << 1;

    // MoveUp
    QTest::newRow("MoveUp (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveUp (3, 0)")
        << 4 << 4 << -1 << -1
        << 3 << 0
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << 2 << 0;

    QTest::newRow("MoveUp (0,1)")
        << 4 << 4 << -1 << -1
        << 0 << 1
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << 0 << 1;

    QTest::newRow("MoveUp, hidden row 1 (2,0)")
        << 4 << 4 << 1 << -1
        << 2 << 0
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveUp, hidden row (1,0)")
        << 4 << 4 << 0 << -1
        << 1 << 0
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << 1 << 0;

    // MoveHome
    QTest::newRow("MoveHome (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveHome) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveHome (3,3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveHome) << int(Qt::NoModifier)
        << 3 << 0;

    QTest::newRow("MoveHome, hidden column 0 (3,3)")
        << 4 << 4 << -1 << 0
        << 3 << 3
        << int(QtTestTableView::MoveHome) << int(Qt::NoModifier)
        << 3 << 1;

    // Use Ctrl modifier
    QTest::newRow("MoveHome + Ctrl (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveHome) << int(Qt::ControlModifier)
        << 0 << 0;

    QTest::newRow("MoveHome + Ctrl (3,3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveHome) << int(Qt::ControlModifier)
        << 0 << 0;

    QTest::newRow("MoveHome + Ctrl, hidden column 0, hidden row 0 (3,3)")
        << 4 << 4 << 0 << 0
        << 3 << 3
        << int(QtTestTableView::MoveHome) << int(Qt::ControlModifier)
        << 1 << 1;

    // MoveEnd
    QTest::newRow("MoveEnd (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::NoModifier)
        << 0 << 3;

    QTest::newRow("MoveEnd (3,3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveEnd) << int(Qt::NoModifier)
        << 3 << 3;

    QTest::newRow("MoveEnd, hidden column (0,0)")
        << 4 << 4 << -1 << 3
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::NoModifier)
        << 0<< 2;

    // Use Ctrl modifier
    QTest::newRow("MoveEnd + Ctrl (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::ControlModifier)
        << 3 << 3;

    QTest::newRow("MoveEnd + Ctrl (3,3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveEnd) << int(Qt::ControlModifier)
        << 3 << 3;

    QTest::newRow("MoveEnd + Ctrl, hidden column 3 (0,0)")
        << 4 << 4 << -1 << 3
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::ControlModifier)
        << 3 << 2;

    QTest::newRow("MoveEnd + Ctrl, hidden column 3, hidden row 3 (0,0)")
        << 4 << 4 << 3 << 3
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::ControlModifier)
        << 2 << 2;

    QTest::newRow("MovePageUp (0,0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MovePageUp) << 0
        << 0 << 0;

    QTest::newRow("MovePageUp (3,3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MovePageUp) << 0
        << 0 << 3;

    QTest::newRow("MovePageDown (3, 3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MovePageDown) << 0
        << 3 << 3;

    QTest::newRow("MovePageDown (0, 3)")
        << 4 << 4 << -1 << -1
        << 0 << 3
        << int(QtTestTableView::MovePageDown) << 0
        << 3 << 3;
}

void tst_QTableView::moveCursor()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, hideRow);
    QFETCH(int, hideColumn);
    QFETCH(int, startRow);
    QFETCH(int, startColumn);
    QFETCH(int, cursorMoveAction);
    QFETCH(int, modifier);
    QFETCH(int, expectedRow);
    QFETCH(int, expectedColumn);

    QtTestTableModel model(rowCount, columnCount);
    QtTestTableView view;

    view.setModel(&model);
    view.hideRow(hideRow);
    view.hideColumn(hideColumn);
    view.show();

    QModelIndex index = model.index(startRow, startColumn);
    view.setCurrentIndex(index);

    QModelIndex newIndex = view.moveCursor((QtTestTableView::CursorAction)cursorMoveAction,
                                           (Qt::KeyboardModifiers)modifier);
    // expected fails, task 119433
    if(newIndex.row() == -1)
        return;
    QCOMPARE(newIndex.row(), expectedRow);
    QCOMPARE(newIndex.column(), expectedColumn);
}

void tst_QTableView::hideRows_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("showRow"); // hide, then show
    QTest::addColumn<int>("hideRow"); // hide only
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("rowSpan");
    QTest::addColumn<int>("columnSpan");

    QTest::newRow("show row 0, hide row 3, no span")
      << 10 << 10
      << 0
      << 3
      << -1 << -1
      << 1 << 1;

    QTest::newRow("show row 0, hide row 3, span")
      << 10 << 10
      << 0
      << 3
      << 0 << 0
      << 3 << 2;
}

void tst_QTableView::hideRows()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, showRow);
    QFETCH(int, hideRow);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(int, rowSpan);
    QFETCH(int, columnSpan);

    QtTestTableModel model(rowCount, columnCount);
    QTableView view;

    view.setModel(&model);
    view.setSpan(row, column, rowSpan, columnSpan);

    view.hideRow(showRow);
    QVERIFY(view.isRowHidden(showRow));

    view.hideRow(hideRow);
    QVERIFY(view.isRowHidden(hideRow));

    view.showRow(showRow);
    QVERIFY(!view.isRowHidden(showRow));
    QVERIFY(view.isRowHidden(hideRow));
}

void tst_QTableView::hideColumns_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("showColumn"); // hide, then show
    QTest::addColumn<int>("hideColumn"); // hide only
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("rowSpan");
    QTest::addColumn<int>("columnSpan");

    QTest::newRow("show col 0, hide col 3, no span")
      << 10 << 10
      << 0
      << 3
      << -1 << -1
      << 1 << 1;

    QTest::newRow("show col 0, hide col 3, span")
      << 10 << 10
      << 0
      << 3
      << 0 << 0
      << 3 << 2;
}

void tst_QTableView::hideColumns()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, showColumn);
    QFETCH(int, hideColumn);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(int, rowSpan);
    QFETCH(int, columnSpan);

    QtTestTableModel model(rowCount, columnCount);

    QTableView view;
    view.setModel(&model);
    view.setSpan(row, column, rowSpan, columnSpan);

    view.hideColumn(showColumn);
    QVERIFY(view.isColumnHidden(showColumn));

    view.hideColumn(hideColumn);
    QVERIFY(view.isColumnHidden(hideColumn));

    view.showColumn(showColumn);
    QVERIFY(!view.isColumnHidden(showColumn));
    QVERIFY(view.isColumnHidden(hideColumn));
}

void tst_QTableView::selection_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("rowSpan");
    QTest::addColumn<int>("columnSpan");
    QTest::addColumn<int>("hideRow");
    QTest::addColumn<int>("hideColumn");
    QTest::addColumn<int>("moveRowFrom");
    QTest::addColumn<int>("moveRowTo");
    QTest::addColumn<int>("moveColumnFrom");
    QTest::addColumn<int>("moveColumnTo");
    QTest::addColumn<int>("rowHeight");
    QTest::addColumn<int>("columnWidth");
    QTest::addColumn<int>("x");
    QTest::addColumn<int>("y");
    QTest::addColumn<int>("width");
    QTest::addColumn<int>("height");
    QTest::addColumn<int>("command");
    QTest::addColumn<int>("selectedCount"); // ### make this more detailed

    QTest::newRow("no span, no hidden, no moved, 3x3 select")
      << 10 << 10                          // dim
      << -1 << -1                          // pos
      << 1 << 1                            // span
      << -1 << -1                          // hide
      << -1 << -1                          // move row
      << -1 << -1                          // move col
      << 40 << 40                          // cell size
      << 20 << 20 << 80 << 80              // rect
      << int(QItemSelectionModel::Select)  // command
      << 9;                                // selected count

    QTest::newRow("row span, no hidden, no moved, 3x3 select")
      << 10 << 10                          // dim
      << 1 << 1                            // pos
      << 2 << 1                            // span
      << -1 << -1                          // hide
      << -1 << -1                          // move row
      << -1 << -1                          // move col
      << 40 << 40                          // cell size
      << 20 << 20 << 80 << 80              // rect
      << int(QItemSelectionModel::Select)  // command
      << 8;                                // selected count

    QTest::newRow("col span, no hidden, no moved, 3x3 select")
      << 10 << 10                          // dim
      << 1 << 1                            // pos
      << 1 << 2                            // span
      << -1 << -1                          // hide
      << -1 << -1                          // move row
      << -1 << -1                          // move col
      << 40 << 40                          // cell size
      << 20 << 20 << 80 << 80              // rect
      << int(QItemSelectionModel::Select)  // command
      << 8;                                // selected count

    QTest::newRow("no span, row hidden, no moved, 3x3 select")
      << 10 << 10                          // dim
      << -1 << -1                          // pos
      << 1 << 1                            // span
      << 1 << -1                           // hide
      << -1 << -1                          // move row
      << -1 << -1                          // move col
      << 40 << 40                          // cell size
      << 20 << 20 << 80 << 80              // rect
      << int(QItemSelectionModel::Select)  // command
      << 9;                                // selected count

    QTest::newRow("no span, col hidden, no moved, 3x3 select")
      << 10 << 10                          // dim
      << -1 << -1                          // pos
      << 1 << 1                            // span
      << -1 << 1                           // hide
      << -1 << -1                          // move row
      << -1 << -1                          // move col
      << 40 << 40                          // cell size
      << 20 << 20 << 80 << 80              // rect
      << int(QItemSelectionModel::Select)  // command
      << 9;                                // selected count

    QTest::newRow("no span, no hidden, row moved, 3x3 select")
      << 10 << 10                          // dim
      << -1 << -1                          // pos
      << 1 << 1                            // span
      << -1 << -1                          // hide
      << 1 << 3                            // move row
      << -1 << -1                          // move col
      << 40 << 40                          // cell size
      << 20 << 20 << 80 << 80              // rect
      << int(QItemSelectionModel::Select)  // command
      << 9;                                // selected count

    QTest::newRow("no span, no hidden, col moved, 3x3 select")
      << 10 << 10                          // dim
      << -1 << -1                          // pos
      << 1 << 1                            // span
      << -1 << -1                          // hide
      << -1 << -1                          // move row
      << 1 << 3                            // move col
      << 40 << 40                          // cell size
      << 20 << 20 << 80 << 80              // rect
      << int(QItemSelectionModel::Select)  // command
      << 9;                                // selected count
}

void tst_QTableView::selection()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(int, rowSpan);
    QFETCH(int, columnSpan);
    QFETCH(int, hideRow);
    QFETCH(int, hideColumn);
    QFETCH(int, moveRowFrom);
    QFETCH(int, moveRowTo);
    QFETCH(int, moveColumnFrom);
    QFETCH(int, moveColumnTo);
    QFETCH(int, rowHeight);
    QFETCH(int, columnWidth);
    QFETCH(int, x);
    QFETCH(int, y);
    QFETCH(int, width);
    QFETCH(int, height);
    QFETCH(int, command);
    QFETCH(int, selectedCount);

    QtTestTableModel model(rowCount, columnCount);

    QtTestTableView view;
    view.show();
    view.setModel(&model);

    view.setSpan(row, column, rowSpan, columnSpan);

    view.hideRow(hideRow);
    view.hideColumn(hideColumn);

    view.verticalHeader()->moveSection(moveRowFrom, moveRowTo);
    view.horizontalHeader()->moveSection(moveColumnFrom, moveColumnTo);

    for (int r = 0; r < rowCount; ++r)
        view.setRowHeight(r, rowHeight);
    for (int c = 0; c < columnCount; ++c)
        view.setColumnWidth(c, columnWidth);

    view.setSelection(QRect(x, y, width, height),
		      QItemSelectionModel::SelectionFlags(command));

    QCOMPARE(view.selectedIndexes().count(), selectedCount);
}

void tst_QTableView::selectRow_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("behavior");
    QTest::addColumn<int>("selectedItems");

    QTest::newRow("SingleSelection and SelectItems")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::SingleSelection
        << (int)QAbstractItemView::SelectItems
        << 0;

    QTest::newRow("SingleSelection and SelectRows")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::SingleSelection
        << (int)QAbstractItemView::SelectRows
        << 10;

    QTest::newRow("SingleSelection and SelectColumns")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::SingleSelection
        << (int)QAbstractItemView::SelectColumns
        << 0;

    QTest::newRow("MultiSelection and SelectItems")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::MultiSelection
        << (int)QAbstractItemView::SelectItems
        << 10;

    QTest::newRow("MultiSelection and SelectRows")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::MultiSelection
        << (int)QAbstractItemView::SelectRows
        << 10;

    QTest::newRow("MultiSelection and SelectColumns")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::MultiSelection
        << (int)QAbstractItemView::SelectColumns
        << 0;

    QTest::newRow("ExtendedSelection and SelectItems")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::ExtendedSelection
        << (int)QAbstractItemView::SelectItems
        << 10;

    QTest::newRow("ExtendedSelection and SelectRows")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::ExtendedSelection
        << (int)QAbstractItemView::SelectRows
        << 10;

    QTest::newRow("ExtendedSelection and SelectColumns")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::ExtendedSelection
        << (int)QAbstractItemView::SelectColumns
        << 0;

    QTest::newRow("ContiguousSelection and SelectItems")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::ContiguousSelection
        << (int)QAbstractItemView::SelectItems
        << 10;

    QTest::newRow("ContiguousSelection and SelectRows")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::ContiguousSelection
        << (int)QAbstractItemView::SelectRows
        << 10;

    QTest::newRow("ContiguousSelection and SelectColumns")
        << 10 << 10
        << 0
        << (int)QAbstractItemView::ContiguousSelection
        << (int)QAbstractItemView::SelectColumns
        << 0;
}

void tst_QTableView::selectRow()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, row);
    QFETCH(int, mode);
    QFETCH(int, behavior);
    QFETCH(int, selectedItems);

    QtTestTableModel model(rowCount, columnCount);
    QTableView view;

    view.setModel(&model);
    view.setSelectionMode((QAbstractItemView::SelectionMode)mode);
    view.setSelectionBehavior((QAbstractItemView::SelectionBehavior)behavior);

    QCOMPARE(view.selectionModel()->selectedIndexes().count(), 0);

    view.selectRow(row);

    //test we have 10 items selected
    QCOMPARE(view.selectionModel()->selectedIndexes().count(), selectedItems);
    //test that all 10 items are in the same row
    for (int i = 0; selectedItems > 0 && i < rowCount; ++i)
        QCOMPARE(view.selectionModel()->selectedIndexes().at(i).row(), row);
}

void tst_QTableView::selectColumn_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("behavior");
    QTest::addColumn<int>("selectedItems");

        QTest::newRow("SingleSelection and SelectItems")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::SingleSelection
            << (int)QAbstractItemView::SelectItems
            << 0;

        QTest::newRow("SingleSelection and SelectRows")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::SingleSelection
            << (int)QAbstractItemView::SelectRows
            << 0;

        QTest::newRow("SingleSelection and SelectColumns")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::SingleSelection
            << (int)QAbstractItemView::SelectColumns
            << 10;

        QTest::newRow("MultiSelection and SelectItems")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::MultiSelection
            << (int)QAbstractItemView::SelectItems
            << 10;

        QTest::newRow("MultiSelection and SelectRows")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::MultiSelection
            << (int)QAbstractItemView::SelectRows
            << 0;

        QTest::newRow("MultiSelection and SelectColumns")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::MultiSelection
            << (int)QAbstractItemView::SelectColumns
            << 10;

        QTest::newRow("ExtendedSelection and SelectItems")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::ExtendedSelection
            << (int)QAbstractItemView::SelectItems
            << 10;

        QTest::newRow("ExtendedSelection and SelectRows")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::ExtendedSelection
            << (int)QAbstractItemView::SelectRows
            << 0;

        QTest::newRow("ExtendedSelection and SelectColumns")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::ExtendedSelection
            << (int)QAbstractItemView::SelectColumns
            << 10;

        QTest::newRow("ContiguousSelection and SelectItems")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::ContiguousSelection
            << (int)QAbstractItemView::SelectItems
            << 10;

        QTest::newRow("ContiguousSelection and SelectRows")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::ContiguousSelection
            << (int)QAbstractItemView::SelectRows
            << 0;

        QTest::newRow("ContiguousSelection and SelectColumns")
            << 10 << 10
            << 0
            << (int)QAbstractItemView::ContiguousSelection
            << (int)QAbstractItemView::SelectColumns
            << 10;
}

void tst_QTableView::selectColumn()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, column);
    QFETCH(int, mode);
    QFETCH(int, behavior);
    QFETCH(int, selectedItems);

    QtTestTableModel model(rowCount, columnCount);

    QTableView view;
    view.setModel(&model);
    view.setSelectionMode((QAbstractItemView::SelectionMode)mode);
    view.setSelectionBehavior((QAbstractItemView::SelectionBehavior)behavior);

    QCOMPARE(view.selectionModel()->selectedIndexes().count(), 0);

    view.selectColumn(column);

    QCOMPARE(view.selectionModel()->selectedIndexes().count(), selectedItems);
    for (int i = 0; selectedItems > 0 && i < columnCount; ++i)
        QCOMPARE(view.selectionModel()->selectedIndexes().at(i).column(), column);
}

Q_DECLARE_METATYPE(QRect)
void tst_QTableView::visualRect_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("hideRow");
    QTest::addColumn<int>("hideColumn");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("rowHeight");
    QTest::addColumn<int>("columnWidth");
    QTest::addColumn<QRect>("expectedRect");

    QTest::newRow("(0,0)")
        << 10 << 10
        << -1 << -1
        << 0 << 0
        << 20 << 30
        << QRect(0, 0, 29, 19);

    QTest::newRow("(0,0) hidden row")
        << 10 << 10
        << 0 << -1
        << 0 << 0
        << 20 << 30
        << QRect();

    QTest::newRow("(0,0) hidden column")
        << 10 << 10
        << -1 << 0
        << 0 << 0
        << 20 << 30
        << QRect();

    QTest::newRow("(0,0) hidden row and column")
        << 10 << 10
        << 0 << 0
        << 0 << 0
        << 20 << 30
        << QRect();

    QTest::newRow("(0,0) out of bounds")
        << 10 << 10
        << -1 << -1
        << 20 << 20
        << 20 << 30
        << QRect();

    QTest::newRow("(5,5), hidden row")
        << 10 << 10
        << 5 << -1
        << 5 << 5
        << 20 << 30
        << QRect();

    QTest::newRow("(9,9)")
        << 10 << 10
        << -1 << -1
        << 9 << 9
        << 20 << 30
        << QRect(30*9, 20*9, 29, 19);
}

void tst_QTableView::visualRect()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, hideRow);
    QFETCH(int, hideColumn);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(int, rowHeight);
    QFETCH(int, columnWidth);
    QFETCH(QRect, expectedRect);

    QtTestTableModel model(rowCount, columnCount);

    QTableView view;
    view.setModel(&model);
    // Make sure that it has 1 pixel between each cell.
    view.setGridStyle(Qt::SolidLine);
    for (int i = 0; i < view.verticalHeader()->count(); ++i)
        view.verticalHeader()->resizeSection(i, rowHeight);
    for (int i = 0; i < view.horizontalHeader()->count(); ++i)
        view.horizontalHeader()->resizeSection(i, columnWidth);

    view.hideRow(hideRow);
    view.hideColumn(hideColumn);

    QRect rect = view.visualRect(model.index(row, column));
    QCOMPARE(rect, expectedRect);
}

void tst_QTableView::fetchMore()
{
    QtTestTableModel model(64, 64);

    model.can_fetch_more = true;

    QTableView view;
    view.setModel(&model);
    view.show();

    QCOMPARE(model.fetch_more_count, 0);
    view.verticalScrollBar()->setValue(view.verticalScrollBar()->maximum());
    QVERIFY(model.fetch_more_count > 0);

    model.fetch_more_count = 0; //reset
    view.scrollToTop();
    QCOMPARE(model.fetch_more_count, 0);

    view.scrollToBottom();
    QVERIFY(model.fetch_more_count > 0);

    model.fetch_more_count = 0; //reset
    view.scrollToTop();
    view.setCurrentIndex(model.index(0, 0));
    QCOMPARE(model.fetch_more_count, 0);

    for (int i = 0; i < 64; ++i)
        QTest::keyClick(&view, Qt::Key_Down);
    QCOMPARE(view.currentIndex(), model.index(63, 0));
    QVERIFY(model.fetch_more_count > 0);
}

void tst_QTableView::setHeaders()
{
    QTableView view;

    // Make sure we don't delete ourselves
    view.setVerticalHeader(view.verticalHeader());
    view.verticalHeader()->count();
    view.setHorizontalHeader(view.horizontalHeader());
    view.horizontalHeader()->count();

    // Try passing around a header without it being deleted
    QTableView view2;
    view2.setVerticalHeader(view.verticalHeader());
    view2.setHorizontalHeader(view.horizontalHeader());
    view.setHorizontalHeader(new QHeaderView(Qt::Horizontal));
    view.setVerticalHeader(new QHeaderView(Qt::Vertical));
    view2.verticalHeader()->count();
    view2.horizontalHeader()->count();

}

void tst_QTableView::resizeRowsToContents_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<bool>("showGrid");
    QTest::addColumn<int>("cellWidth");
    QTest::addColumn<int>("cellHeight");
    QTest::addColumn<int>("rowHeight");
    QTest::addColumn<int>("columnWidth");

    QTest::newRow("10x10 grid shown 40x40")
        << 10 << 10 << false << 40 << 40 << 40 << 40;

    QTest::newRow("10x10 grid not shown 40x40")
        << 10 << 10 << true << 40 << 40 << 41 << 41;
}

void tst_QTableView::resizeRowsToContents()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(bool, showGrid);
    QFETCH(int, cellWidth);
    QFETCH(int, cellHeight);
    QFETCH(int, rowHeight);
    QFETCH(int, columnWidth);
    Q_UNUSED(columnWidth);

    QtTestTableModel model(rowCount, columnCount);
    QtTestTableView view;
    QtTestItemDelegate delegate;

    view.setModel(&model);
    view.setItemDelegate(&delegate);
    view.setShowGrid(showGrid); // the grid will add to the row height

    delegate.hint = QSize(cellWidth, cellHeight);

    QSignalSpy resizedSpy(view.verticalHeader(), SIGNAL(sectionResized(int, int, int)));
    view.resizeRowsToContents();

    QCOMPARE(resizedSpy.count(), model.rowCount());
    for (int r = 0; r < model.rowCount(); ++r)
        QCOMPARE(view.rowHeight(r), rowHeight);
}

void tst_QTableView::resizeColumnsToContents_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<bool>("showGrid");
    QTest::addColumn<int>("cellWidth");
    QTest::addColumn<int>("cellHeight");
    QTest::addColumn<int>("rowHeight");
    QTest::addColumn<int>("columnWidth");

    QTest::newRow("10x10 grid shown 40x40")
        << 10 << 10 << false << 40 << 40 << 40 << 40;

    QTest::newRow("10x10 grid not shown 40x40")
        << 10 << 10 << true << 40 << 40 << 41 << 41;
}

void tst_QTableView::resizeColumnsToContents()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(bool, showGrid);
    QFETCH(int, cellWidth);
    QFETCH(int, cellHeight);
    QFETCH(int, rowHeight);
    QFETCH(int, columnWidth);
    Q_UNUSED(rowHeight);

    QtTestTableModel model(rowCount, columnCount);
    QtTestTableView view;
    QtTestItemDelegate delegate;

    view.setModel(&model);
    view.setItemDelegate(&delegate);
    view.setShowGrid(showGrid); // the grid will add to the row height

    delegate.hint = QSize(cellWidth, cellHeight);

    QSignalSpy resizedSpy(view.horizontalHeader(), SIGNAL(sectionResized(int, int, int)));
    view.resizeColumnsToContents();

    QCOMPARE(resizedSpy.count(), model.columnCount());
    for (int c = 0; c < model.columnCount(); ++c)
        QCOMPARE(view.columnWidth(c), columnWidth);
}

void tst_QTableView::rowViewportPosition_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("rowHeight");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("verticalScrollMode");
    QTest::addColumn<int>("verticalScrollValue");
    QTest::addColumn<int>("rowViewportPosition");

    QTest::newRow("row 0, scroll per item 0")
        << 10 << 40 << 0 << int(QAbstractItemView::ScrollPerItem) << 0 << 0;

    QTest::newRow("row 1, scroll per item, 0")
        << 10 << 40 << 1 << int(QAbstractItemView::ScrollPerItem) << 0 << 1 * 40;

    QTest::newRow("row 1, scroll per item, 1")
        << 10 << 40 << 1 << int(QAbstractItemView::ScrollPerItem) << 1 << 0;

    QTest::newRow("row 5, scroll per item, 0")
        << 10 << 40 << 5 << int(QAbstractItemView::ScrollPerItem) << 0 << 5 * 40;

    QTest::newRow("row 5, scroll per item, 5")
        << 10 << 40 << 5 << int(QAbstractItemView::ScrollPerItem) << 5 << 0;

    QTest::newRow("row 9, scroll per item, 0")
        << 10 << 40 << 9 << int(QAbstractItemView::ScrollPerItem) << 0 << 9 * 40;

    QTest::newRow("row 9, scroll per item, 5")
        << 10 << 40 << 9 << int(QAbstractItemView::ScrollPerItem) << 5 << 4 * 40;

    QTest::newRow("row 0, scroll per pixel 0")
        << 10 << 40 << 0 << int(QAbstractItemView::ScrollPerPixel) << 0 << 0;

    QTest::newRow("row 1, scroll per pixel, 0")
        << 10 << 40 << 1 << int(QAbstractItemView::ScrollPerPixel) << 0 << 1 * 40;

    QTest::newRow("row 1, scroll per pixel, 1")
        << 10 << 40 << 1 << int(QAbstractItemView::ScrollPerPixel) << 1 * 40 << 0;

    QTest::newRow("row 5, scroll per pixel, 0")
        << 10 << 40 << 5 << int(QAbstractItemView::ScrollPerPixel) << 0 << 5 * 40;

    QTest::newRow("row 5, scroll per pixel, 5")
        << 10 << 40 << 5 << int(QAbstractItemView::ScrollPerPixel) << 5 * 40 << 0;

    QTest::newRow("row 9, scroll per pixel, 0")
        << 10 << 40 << 9 << int(QAbstractItemView::ScrollPerPixel) << 0 << 9 * 40;

    QTest::newRow("row 9, scroll per pixel, 5")
        << 10 << 40 << 9 << int(QAbstractItemView::ScrollPerPixel) << 5 * 40 << 4 * 40;
}

void tst_QTableView::rowViewportPosition()
{
    QFETCH(int, rowCount);
    QFETCH(int, rowHeight);
    QFETCH(int, row);
    QFETCH(int, verticalScrollMode);
    QFETCH(int, verticalScrollValue);
    QFETCH(int, rowViewportPosition);

    QtTestTableModel model(rowCount, 1);
    QtTestTableView view;
    view.resize(100, 2 * rowHeight);
    view.show();

    view.setModel(&model);
    for (int r = 0; r < rowCount; ++r)
        view.setRowHeight(r, rowHeight);

    view.setVerticalScrollMode((QAbstractItemView::ScrollMode)verticalScrollMode);
    view.verticalScrollBar()->setValue(verticalScrollValue);

    QCOMPARE(view.rowViewportPosition(row), rowViewportPosition);
}

void tst_QTableView::rowAt_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("rowHeight");
    QTest::addColumn<IntList>("hiddenRows");
    QTest::addColumn<int>("coordinate");
    QTest::addColumn<int>("row");

    QTest::newRow("row at 100") << 5 << 40 << IntList() << 100 << 2;
    QTest::newRow("row at 180") << 5 << 40 << IntList() << 180 << 4;
    QTest::newRow("row at 20")  << 5 << 40 << IntList() <<  20 << 0;

    // ### expand the dataset to include hidden rows
}

void tst_QTableView::rowAt()
{
    QFETCH(int, rowCount);
    QFETCH(int, rowHeight);
    QFETCH(IntList, hiddenRows);
    QFETCH(int, coordinate);
    QFETCH(int, row);

    QtTestTableModel model(rowCount, 1);
    QtTestTableView view;
    view.resize(100, 2 * rowHeight);

    view.setModel(&model);

    for (int r = 0; r < rowCount; ++r)
        view.setRowHeight(r, rowHeight);

    for (int i = 0; i < hiddenRows.count(); ++i)
        view.hideRow(hiddenRows.at(i));

    QCOMPARE(view.rowAt(coordinate), row);
}

void tst_QTableView::rowHeight_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<IntList>("rowHeights");
    QTest::addColumn<BoolList>("hiddenRows");

    QTest::newRow("increasing")
      << 5
      << (IntList() << 20 << 30 << 40 << 50 << 60)
      << (BoolList() << false << false << false << false << false);

    QTest::newRow("decreasing")
      << 5
      << (IntList() << 60 << 50 << 40 << 30 << 20)
      << (BoolList() << false << false << false << false << false);

    QTest::newRow("random")
      << 5
      << (IntList() << 87 << 34 << 68 << 91 << 27)
      << (BoolList() << false << false << false << false << false);

    // ### expand the dataset to include hidden rows
}

void tst_QTableView::rowHeight()
{
    QFETCH(int, rowCount);
    QFETCH(IntList, rowHeights);
    QFETCH(BoolList, hiddenRows);

    QtTestTableModel model(rowCount, 1);
    QtTestTableView view;

    view.setModel(&model);

    for (int r = 0; r < rowCount; ++r) {
        view.setRowHeight(r, rowHeights.at(r));
        view.setRowHidden(r, hiddenRows.at(r));
    }

    for (int r = 0; r < rowCount; ++r) {
        if (hiddenRows.at(r))
            QCOMPARE(view.rowHeight(r), 0);
        else
            QCOMPARE(view.rowHeight(r), rowHeights.at(r));
    }
}

void tst_QTableView::columnViewportPosition_data()
{
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("columnWidth");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("horizontalScrollMode");
    QTest::addColumn<int>("horizontalScrollValue");
    QTest::addColumn<int>("columnViewportPosition");

    QTest::newRow("column 0, scroll per item 0")
        << 10 << 40 << 0 << int(QAbstractItemView::ScrollPerItem) << 0 << 0;

    QTest::newRow("column 1, scroll per item, 0")
        << 10 << 40 << 1 << int(QAbstractItemView::ScrollPerItem) << 0 << 1 * 40;

    QTest::newRow("column 1, scroll per item, 1")
        << 10 << 40 << 1 << int(QAbstractItemView::ScrollPerItem) << 1 << 0;

    QTest::newRow("column 5, scroll per item, 0")
        << 10 << 40 << 5 << int(QAbstractItemView::ScrollPerItem) << 0 << 5 * 40;

    QTest::newRow("column 5, scroll per item, 5")
        << 10 << 40 << 5 << int(QAbstractItemView::ScrollPerItem) << 5 << 0;

    QTest::newRow("column 9, scroll per item, 0")
        << 10 << 40 << 9 << int(QAbstractItemView::ScrollPerItem) << 0 << 9 * 40;

    QTest::newRow("column 9, scroll per item, 5")
        << 10 << 40 << 9 << int(QAbstractItemView::ScrollPerItem) << 5 << 4 * 40;

    QTest::newRow("column 0, scroll per pixel 0")
        << 10 << 40 << 0 << int(QAbstractItemView::ScrollPerPixel) << 0 << 0;

    QTest::newRow("column 1, scroll per pixel 0")
        << 10 << 40 << 1 << int(QAbstractItemView::ScrollPerPixel) << 0 << 1 * 40;

    QTest::newRow("column 1, scroll per pixel 1")
        << 10 << 40 << 1 << int(QAbstractItemView::ScrollPerPixel) << 1 * 40 << 0;

    QTest::newRow("column 5, scroll per pixel 0")
        << 10 << 40 << 5 << int(QAbstractItemView::ScrollPerPixel) << 0 << 5 * 40;

    QTest::newRow("column 5, scroll per pixel 5")
        << 10 << 40 << 5 << int(QAbstractItemView::ScrollPerPixel) << 5 * 40 << 0;

    QTest::newRow("column 9, scroll per pixel 0")
        << 10 << 40 << 9 << int(QAbstractItemView::ScrollPerPixel) << 0 << 9 * 40;

    QTest::newRow("column 9, scroll per pixel 5")
        << 10 << 40 << 9 << int(QAbstractItemView::ScrollPerPixel) << 5 * 40 << 4 * 40;
}

void tst_QTableView::columnViewportPosition()
{
    QFETCH(int, columnCount);
    QFETCH(int, columnWidth);
    QFETCH(int, column);
    QFETCH(int, horizontalScrollMode);
    QFETCH(int, horizontalScrollValue);
    QFETCH(int, columnViewportPosition);

    QtTestTableModel model(1, columnCount);
    QtTestTableView view;
    view.resize(2 * columnWidth, 100);
    view.show();

    view.setModel(&model);
    for (int c = 0; c < columnCount; ++c)
        view.setColumnWidth(c, columnWidth);

    view.setHorizontalScrollMode((QAbstractItemView::ScrollMode)horizontalScrollMode);
    view.horizontalScrollBar()->setValue(horizontalScrollValue);

    QCOMPARE(view.columnViewportPosition(column), columnViewportPosition);
}

void tst_QTableView::columnAt_data()
{
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("columnWidth");
    QTest::addColumn<IntList>("hiddenColumns");
    QTest::addColumn<int>("coordinate");
    QTest::addColumn<int>("column");

    QTest::newRow("column at 100") << 5 << 40 << IntList() << 100 << 2;
    QTest::newRow("column at 180") << 5 << 40 << IntList() << 180 << 4;
    QTest::newRow("column at 20")  << 5 << 40 << IntList() <<  20 << 0;

    // ### expand the dataset to include hidden coumns
}

void tst_QTableView::columnAt()
{
    QFETCH(int, columnCount);
    QFETCH(int, columnWidth);
    QFETCH(IntList, hiddenColumns);
    QFETCH(int, coordinate);
    QFETCH(int, column);

    QtTestTableModel model(1, columnCount);
    QtTestTableView view;
    view.resize(2 * columnWidth, 100);

    view.setModel(&model);

    for (int c = 0; c < columnCount; ++c)
        view.setColumnWidth(c, columnWidth);

    for (int i = 0; i < hiddenColumns.count(); ++i)
        view.hideColumn(hiddenColumns.at(i));

    QCOMPARE(view.columnAt(coordinate), column);
}

void tst_QTableView::columnWidth_data()
{
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<IntList>("columnWidths");
    QTest::addColumn<BoolList>("hiddenColumns");

    QTest::newRow("increasing")
      << 5
      << (IntList() << 20 << 30 << 40 << 50 << 60)
      << (BoolList() << false << false << false << false << false);

    QTest::newRow("decreasing")
      << 5
      << (IntList() << 60 << 50 << 40 << 30 << 20)
      << (BoolList() << false << false << false << false << false);

    QTest::newRow("random")
      << 5
      << (IntList() << 87 << 34 << 68 << 91 << 27)
      << (BoolList() << false << false << false << false << false);

    // ### expand the dataset to include hidden columns
}

void tst_QTableView::columnWidth()
{
    QFETCH(int, columnCount);
    QFETCH(IntList, columnWidths);
    QFETCH(BoolList, hiddenColumns);

    QtTestTableModel model(1, columnCount);
    QtTestTableView view;

    view.setModel(&model);

    for (int c = 0; c < columnCount; ++c) {
        view.setColumnWidth(c, columnWidths.at(c));
        view.setColumnHidden(c, hiddenColumns.at(c));
    }

    for (int c = 0; c < columnCount; ++c) {
        if (hiddenColumns.at(c))
            QCOMPARE(view.columnWidth(c), 0);
        else
            QCOMPARE(view.columnWidth(c), columnWidths.at(c));
    }
}

void tst_QTableView::hiddenRow_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<BoolList>("hiddenRows");

    QTest::newRow("first hidden")
      << 5 << (BoolList() << true << false << false << false << false);

    QTest::newRow("last hidden")
      << 5 << (BoolList() << false << false << false << false << true);

    QTest::newRow("none hidden")
      << 5 << (BoolList() << false << false << false << false << false);

    QTest::newRow("all hidden")
      << 5 << (BoolList() << true << true << true << true << true);
 }

void tst_QTableView::hiddenRow()
{
    QFETCH(int, rowCount);
    QFETCH(BoolList, hiddenRows);


    QtTestTableModel model(rowCount, 1);
    QtTestTableView view;

    view.setModel(&model);

    for (int r = 0; r < rowCount; ++r)
        QVERIFY(!view.isRowHidden(r));

    for (int r = 0; r < rowCount; ++r)
        view.setRowHidden(r, hiddenRows.at(r));

    for (int r = 0; r < rowCount; ++r)
        QCOMPARE(view.isRowHidden(r), hiddenRows.at(r));

    for (int r = 0; r < rowCount; ++r)
        view.setRowHidden(r, false);

    for (int r = 0; r < rowCount; ++r)
        QVERIFY(!view.isRowHidden(r));
}

void tst_QTableView::hiddenColumn_data()
{
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<BoolList>("hiddenColumns");

    QTest::newRow("first hidden")
      << 5 << (BoolList() << true << false << false << false << false);

    QTest::newRow("last hidden")
      << 5 << (BoolList() << false << false << false << false << true);

    QTest::newRow("none hidden")
      << 5 << (BoolList() << false << false << false << false << false);

    QTest::newRow("all hidden")
      << 5 << (BoolList() << true << true << true << true << true);
}

void tst_QTableView::hiddenColumn()
{
    QFETCH(int, columnCount);
    QFETCH(BoolList, hiddenColumns);

    QtTestTableModel model(1, columnCount);
    QtTestTableView view;

    view.setModel(&model);

    for (int c = 0; c < columnCount; ++c)
        QVERIFY(!view.isColumnHidden(c));

    for (int c = 0; c < columnCount; ++c)
        view.setColumnHidden(c, hiddenColumns.at(c));

    for (int c = 0; c < columnCount; ++c)
        QCOMPARE(view.isColumnHidden(c), hiddenColumns.at(c));

    for (int c = 0; c < columnCount; ++c)
        view.setColumnHidden(c, false);

    for (int c = 0; c < columnCount; ++c)
        QVERIFY(!view.isColumnHidden(c));
}

void tst_QTableView::sortingEnabled_data()
{
//    QTest::addColumn<int>("columnCount");
}

void tst_QTableView::sortingEnabled()
{
//    QFETCH(int, columnCount);
}

void tst_QTableView::scrollTo_data()
{
    QTest::addColumn<int>("verticalScrollMode");
    QTest::addColumn<int>("horizontalScrollMode");
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("rowHeight");
    QTest::addColumn<int>("columnWidth");
    QTest::addColumn<int>("hiddenRow");
    QTest::addColumn<int>("hiddenColumn");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("rowSpan");
    QTest::addColumn<int>("columnSpan");
    QTest::addColumn<int>("horizontalScroll");
    QTest::addColumn<int>("verticalScroll");
    QTest::addColumn<int>("scrollHint");
    QTest::addColumn<int>("expectedHorizontalScroll");
    QTest::addColumn<int>("expectedVerticalScroll");

    QTest::newRow("no hidden, no span, no scroll, per item")
        << (int)QAbstractItemView::ScrollPerItem
        << (int)QAbstractItemView::ScrollPerItem
        << 10 << 10  // table
        << 40 << 40  // size
        << -1 << -1  // hide
        << 0 << 0    // cell
        << 1 << 1    // span
        << 0 << 0    // scroll
        << (int)QAbstractItemView::PositionAtTop
        << 0 << 0;   // expected

    QTest::newRow("no hidden, no span, no scroll, per pixel")
        << (int)QAbstractItemView::ScrollPerPixel
        << (int)QAbstractItemView::ScrollPerPixel
        << 10 << 10  // table
        << 40 << 40  // size
        << -1 << -1  // hide
        << 0 << 0    // cell
        << 1 << 1    // span
        << 0 << 0    // scroll
        << (int)QAbstractItemView::PositionAtTop
        << 0 << 0;   // expected

    QTest::newRow("hidden, no span, no scroll, per item")
        << (int)QAbstractItemView::ScrollPerItem
        << (int)QAbstractItemView::ScrollPerItem
        << 10 << 10  // table
        << 40 << 40  // size
        << 3 << 3    // hide
        << 5 << 5    // cell
        << 1 << 1    // span
        << 0 << 0    // scroll
        << (int)QAbstractItemView::PositionAtTop
        << 4 << 4;   // expected
}

void tst_QTableView::scrollTo()
{
    QFETCH(int, horizontalScrollMode);
    QFETCH(int, verticalScrollMode);
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, rowHeight);
    QFETCH(int, columnWidth);
    QFETCH(int, hiddenRow);
    QFETCH(int, hiddenColumn);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(int, rowSpan);
    QFETCH(int, columnSpan);
    QFETCH(int, horizontalScroll);
    QFETCH(int, verticalScroll);
    QFETCH(int, scrollHint);
    QFETCH(int, expectedHorizontalScroll);
    QFETCH(int, expectedVerticalScroll);

    QtTestTableModel model(rowCount, columnCount);
    QtTestTableView view;

    view.show();
    view.resize(columnWidth * 2, rowHeight * 2);
    view.setModel(&model);
    view.setSpan(row, column, rowSpan, columnSpan);
    view.hideRow(hiddenRow);
    view.hideColumn(hiddenColumn);
    view.setHorizontalScrollMode((QAbstractItemView::ScrollMode)horizontalScrollMode);
    view.setVerticalScrollMode((QAbstractItemView::ScrollMode)verticalScrollMode);

    for (int r = 0; r < rowCount; ++r)
        view.setRowHeight(r, rowHeight);
    for (int c = 0; c < columnCount; ++c)
        view.setColumnWidth(c, columnWidth);

    QTest::qWait(0); // ### needed to pass the test
    view.horizontalScrollBar()->setValue(horizontalScroll);
    view.verticalScrollBar()->setValue(verticalScroll);

    QModelIndex index = model.index(row, column);
    QVERIFY(index.isValid());
    view.scrollTo(index, (QAbstractItemView::ScrollHint)scrollHint);
    QCOMPARE(view.horizontalScrollBar()->value(), expectedHorizontalScroll);
    QCOMPARE(view.verticalScrollBar()->value(), expectedVerticalScroll);
}

void tst_QTableView::indexAt_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("rowHeight");
    QTest::addColumn<int>("columnWidth");
    QTest::addColumn<int>("hiddenRow");
    QTest::addColumn<int>("hiddenColumn");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("rowSpan");
    QTest::addColumn<int>("columnSpan");
    QTest::addColumn<int>("horizontalScroll");
    QTest::addColumn<int>("verticalScroll");
    QTest::addColumn<int>("x");
    QTest::addColumn<int>("y");
    QTest::addColumn<int>("expectedRow");
    QTest::addColumn<int>("expectedColumn");

    QTest::newRow("no hidden, no span, no scroll, (20,20)")
      << 10 << 10  // dim
      << 40 << 40  // size
      << -1 << -1  // hide
      << -1 << -1  // pos
      << 1 << 1    // span
      << 0 << 0    // scroll
      << 20 << 20  // point
      << 0 << 0;   // expected

    QTest::newRow("row hidden, no span, no scroll, at (20,20)")
      << 10 << 10  // dim
      << 40 << 40  // size
      << 0 << -1   // hide
      << -1 << -1  // pos
      << 1 << 1    // span
      << 0 << 0    // scroll
      << 20 << 20  // point
      << 1 << 0;   // expected

    QTest::newRow("col hidden, no span, no scroll, at (20,20)")
      << 10 << 10  // dim
      << 40 << 40  // size
      << -1 << 0   // hide
      << -1 << -1  // pos
      << 1 << 1    // span
      << 0 << 0    // scroll
      << 20 << 20  // point
      << 0 << 1;   // expected

    QTest::newRow("no hidden, row span, no scroll, at (60,20)")
      << 10 << 10  // dim
      << 40 << 40  // size
      << -1 << -1  // hide
      << 0 << 0    // pos
      << 2 << 1    // span
      << 0 << 0    // scroll
      << 20 << 60  // point
      << 0 << 0;   // expected


    QTest::newRow("no hidden, col span, no scroll, at (60,20)")
      << 10 << 10  // dim
      << 40 << 40  // size
      << -1 << -1  // hide
      << 0 << 0    // pos
      << 1 << 2    // span
      << 0 << 0    // scroll
      << 60 << 20  // point
      << 0 << 0;   // expected

    QTest::newRow("no hidden, no span, scroll (5,0), at (20,20)")
      << 10 << 10  // dim
      << 40 << 40  // size
      << -1 << -1  // hide
      << -1 << -1  // pos
      << 1 << 1    // span
      << 5 << 0    // scroll
      << 20 << 20  // point
      << 0 << 5;   // expected

    QTest::newRow("no hidden, no span, scroll (0,5), at (20,20)")
      << 10 << 10  // dim
      << 40 << 40  // size
      << -1 << -1  // hide
      << -1 << -1  // pos
      << 1 << 1    // span
      << 0 << 5    // scroll
      << 20 << 20  // point
      << 5 << 0;   // expected

    QTest::newRow("no hidden, no span, scroll (5,5), at (20,20)")
      << 10 << 10  // dim
      << 40 << 40  // size
      << -1 << -1  // hide
      << -1 << -1  // pos
      << 1 << 1    // span
      << 5 << 5    // scroll
      << 20 << 20  // point
      << 5 << 5;   // expected
}

void tst_QTableView::indexAt()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, rowHeight);
    QFETCH(int, columnWidth);
    QFETCH(int, hiddenRow);
    QFETCH(int, hiddenColumn);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(int, rowSpan);
    QFETCH(int, columnSpan);
    QFETCH(int, horizontalScroll);
    QFETCH(int, verticalScroll);
    QFETCH(int, x);
    QFETCH(int, y);
    QFETCH(int, expectedRow);
    QFETCH(int, expectedColumn);

    QtTestTableModel model(rowCount, columnCount);
    QtTestTableView view;

    view.show();
    view.setModel(&model);
    view.setSpan(row, column, rowSpan, columnSpan);
    view.hideRow(hiddenRow);
    view.hideColumn(hiddenColumn);

    for (int r = 0; r < rowCount; ++r)
        view.setRowHeight(r, rowHeight);
    for (int c = 0; c < columnCount; ++c)
        view.setColumnWidth(c, columnWidth);

    QTest::qWait(0); // ### needed to pass the test
    view.horizontalScrollBar()->setValue(horizontalScroll);
    view.verticalScrollBar()->setValue(verticalScroll);

    QModelIndex index = view.indexAt(QPoint(x, y));
    QCOMPARE(index.row(), expectedRow);
    QCOMPARE(index.column(), expectedColumn);
}

void tst_QTableView::span_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("hiddenRow");
    QTest::addColumn<int>("hiddenColumn");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("rowSpan");
    QTest::addColumn<int>("columnSpan");
    QTest::addColumn<int>("expectedRowSpan");
    QTest::addColumn<int>("expectedColumnSpan");

    QTest::newRow("top left 2x2")
      << 10 << 10
      << -1 << -1
      << 0 << 0
      << 2 << 2
      << 2 << 2;

    QTest::newRow("top left 1x2")
      << 10 << 10
      << 3 << 3
      << 0 << 0
      << 1 << 2
      << 1 << 2;

    QTest::newRow("top left 2x1")
      << 10 << 10
      << -1 << -1
      << 0 << 0
      << 2 << 1
      << 2 << 1;

    QTest::newRow("top left 2x0")
      << 10 << 10
      << -1 << -1
      << 0 << 0
      << 2 << 0
      << 2 << 0;

    QTest::newRow("top left 0x2")
      << 10 << 10
      << -1 << -1
      << 0 << 0
      << 0 << 2
      << 0 << 2;

    QTest::newRow("invalid 2x2")
      << 10 << 10
      << -1 << -1
      << -1 << -1
      << 2 << 2
      << 2 << 2;

    QTest::newRow("top left 2x2")
      << 10 << 10
      << -1 << -1
      << 0 << 0
      << 2 << 2
      << 2 << 2;

    QTest::newRow("bottom right 2x2")
      << 10 << 10
      << -1 << -1
      << 8 << 8
      << 2 << 2
      << 2 << 2;
}

void tst_QTableView::span()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, hiddenRow);
    QFETCH(int, hiddenColumn);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(int, rowSpan);
    QFETCH(int, columnSpan);
    QFETCH(int, expectedRowSpan);
    QFETCH(int, expectedColumnSpan);

    QtTestTableModel model(rowCount, columnCount);
    QtTestTableView view;

    view.setModel(&model);
    view.setSpan(row, column, rowSpan, columnSpan);
    view.hideRow(hiddenRow);
    view.hideColumn(hiddenColumn);
    view.show();

    QCOMPARE(view.rowSpan(row, column), expectedRowSpan);
    QCOMPARE(view.columnSpan(row, column), expectedColumnSpan);

    if (hiddenRow > -1) {
        QModelIndex hidden = model.index(hiddenRow, columnCount - 1);
	QVERIFY(view.isIndexHidden(hidden));
    }

    if (hiddenColumn > -1) {
        QModelIndex hidden = model.index(rowCount - 1, hiddenColumn);
        QVERIFY(view.isIndexHidden(hidden));
    }

    // a comment
}


class Model : public QAbstractTableModel {

Q_OBJECT

public:
    Model(QObject * parent = 0) : QAbstractTableModel(parent) {
    }

    int rowCount(const QModelIndex &) const {
        return rows;
    }
    int columnCount(const QModelIndex &) const {
        return columns;
    }
    QVariant data(const QModelIndex &index, int role) const
    {
        Q_UNUSED(index);
        Q_UNUSED(role);
        return QVariant();
    }
    void res() { reset(); }

    int rows;
    int columns;
};

void tst_QTableView::checkHeaderReset()
{
    QTableView view;
    Model m;
    m.rows = 3;
    m.columns = 3;
    view.setModel(&m);

    m.rows = 4;
    m.columns = 4;
    m.res();
    QCOMPARE(view.horizontalHeader()->count(), 4);
}

void tst_QTableView::checkHeaderMinSize()
{
    //tests if the minimumsize is of a header is taken into account
    //while computing QTableView geometry. For that we test the position of the
    //viewport.
    QTableView view;
    QStringListModel m;
    m.setStringList( QStringList() << QLatin1String("one cell is enough"));
    view.setModel(&m);

    //setting the minimum height on the horizontal header
    //and the minimum width on the vertical header
    view.horizontalHeader()->setMinimumHeight(50);
    view.verticalHeader()->setMinimumWidth(100);

    view.show();

    QVERIFY( view.verticalHeader()->y() >= view.horizontalHeader()->minimumHeight());
    QVERIFY( view.horizontalHeader()->x() >= view.verticalHeader()->minimumWidth());
}

void tst_QTableView::tabFocus()
{
    // QTableView enables tabKeyNavigation by default, but you should be able
    // to change focus on an empty table view, or on a table view that doesn't
    // have this property set.
    QWidget window;

    QTableView *view = new QTableView(&window);
    QLineEdit *edit = new QLineEdit(&window);

    window.show();
//wait for window manager:
#ifdef Q_WS_QWS
    qApp->processEvents();
#endif
    window.setFocus();

    qApp->processEvents();

    WAIT_FOR_CONDITION(window.hasFocus(), true);

    qApp->processEvents();

    // window
    QVERIFY(window.hasFocus());
    QVERIFY(!view->hasFocus());
    QVERIFY(!edit->hasFocus());

    for (int i = 0; i < 2; ++i) {
        // tab to view
        QTest::keyPress(qApp->focusWidget(), Qt::Key_Tab);
        QVERIFY(!window.hasFocus());
        QVERIFY(view->hasFocus());
        QVERIFY(!edit->hasFocus());

        // tab to edit
        QTest::keyPress(qApp->focusWidget(), Qt::Key_Tab);
        QVERIFY(!window.hasFocus());
        QVERIFY(!view->hasFocus());
        QVERIFY(edit->hasFocus());
    }

    // backtab to view
    QTest::keyPress(qApp->focusWidget(), Qt::Key_Backtab);
    QVERIFY(!window.hasFocus());
    QVERIFY(view->hasFocus());
    QVERIFY(!edit->hasFocus());

    // backtab to edit
    QTest::keyPress(qApp->focusWidget(), Qt::Key_Backtab);
    QVERIFY(!window.hasFocus());
    QVERIFY(!view->hasFocus());
    QVERIFY(edit->hasFocus());

    QStandardItemModel *model = new QStandardItemModel;
    view->setModel(model);

    // backtab to view
    QTest::keyPress(qApp->focusWidget(), Qt::Key_Backtab);
    QVERIFY(!window.hasFocus());
    QVERIFY(view->hasFocus());
    QVERIFY(!edit->hasFocus());

    // backtab to edit
    QTest::keyPress(qApp->focusWidget(), Qt::Key_Backtab);
    QVERIFY(!window.hasFocus());
    QVERIFY(!view->hasFocus());
    QVERIFY(edit->hasFocus());

    model->insertRow(0, new QStandardItem("Hei"));
    model->insertRow(0, new QStandardItem("Hei"));
    model->insertRow(0, new QStandardItem("Hei"));

    // backtab to view
    QTest::keyPress(qApp->focusWidget(), Qt::Key_Backtab);
    QVERIFY(!window.hasFocus());
    QVERIFY(view->hasFocus());
    QVERIFY(!edit->hasFocus());

    // backtab to edit doesn't work
    QTest::keyPress(qApp->focusWidget(), Qt::Key_Backtab);
    QVERIFY(!window.hasFocus());
    QVERIFY(view->hasFocus());
    QVERIFY(!edit->hasFocus());

    view->setTabKeyNavigation(false);

    // backtab to edit
    QTest::keyPress(qApp->focusWidget(), Qt::Key_Backtab);
    QVERIFY(!window.hasFocus());
    QVERIFY(!view->hasFocus());
    QVERIFY(edit->hasFocus());

    delete model;
}

QTEST_MAIN(tst_QTableView)
#include "tst_qtableview.moc"
