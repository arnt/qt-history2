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
    void headerSections_unhideRow();

    void moveCursor2();

    void moveCursor_data();
    void moveCursor();

    void hideRows();
    void hideColumns();

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
    void columnViewportPosiiton();

    void columnAt_data();
    void columnAt();

    void columnWidth_data();
    void columnWidth();

    void hiddeRow_data();
    void hiddeRow();

    void hiddeColumn_data();
    void hiddnColumn();

    void sortingEnabled_data();
    void sortingEnabled();

    void scrollTo_data();
    void scrollTo();

    void indexAt_data();
    void indexAt();

    void rowSpan_data();
    void rowSpan();
    
    void columnSpan_data();
    void columnSpan();
};

// Testing get/set functions
void tst_QTableView::getSetCheck()
{
    QTableView obj1;
    QHeaderView *var1 = new QHeaderView(Qt::Horizontal);
    obj1.setHorizontalHeader(var1);
    QCOMPARE(var1, obj1.horizontalHeader());
#if QT_VERSION >= 0x040200
    obj1.setHorizontalHeader((QHeaderView *)0);
    QCOMPARE(var1, obj1.horizontalHeader());
#endif
    delete var1;

    QHeaderView *var2 = new QHeaderView(Qt::Vertical);
    obj1.setVerticalHeader(var2);
    QCOMPARE(var2, obj1.verticalHeader());
#if QT_VERSION >= 0x040200
    obj1.setVerticalHeader((QHeaderView *)0);
    QCOMPARE(var2, obj1.verticalHeader());
#endif
    delete var2;

    obj1.setShowGrid(false);
    QCOMPARE(false, obj1.showGrid());
    obj1.setShowGrid(true);
    QCOMPARE(true, obj1.showGrid());
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

    QTest::newRow("2x2 model") << 2 << 2;
    QTest::newRow("10x10 model") << 10  << 10;
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

    QTest::newRow("2x2 model") << 2 << 2;
    QTest::newRow("10x10 model") << 10  << 10;
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

}

void tst_QTableView::keyboardNavigation()
{
    int rows = 16;
    int columns = 16;

    QStandardItemModel model(rows, columns);
    QTableView view;
    view.setModel(&model);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QModelIndex index = model.index(i, j);
            model.setData(index, QString("[%1,%2]").arg(i).arg(j));
            view.setCurrentIndex(index);
            QApplication::instance()->processEvents();
            QCOMPARE(view.currentIndex(), index);
        }
    }
    view.show();

    QVector<Qt::Key> keymoves;
    keymoves << Qt::Key_Up << Qt::Key_Up << Qt::Key_Right << Qt::Key_Right << Qt::Key_Up
             << Qt::Key_Left << Qt::Key_Left << Qt::Key_Up << Qt::Key_Down << Qt::Key_Up
             << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up
             << Qt::Key_Left << Qt::Key_Left << Qt::Key_Up << Qt::Key_Down;

    int row = rows - 1;
    int column = columns - 1;
    QModelIndex index = model.index(row, column);
    view.setCurrentIndex(index);
    QApplication::instance()->processEvents();
    for (int i = 0; i < keymoves.size(); ++i) {
        Qt::Key key = keymoves.at(i);
        QTest::keyClick(&view, key);
        switch (key) {
        case Qt::Key_Up:
            row = qMax(0, row - 1);
            break;
        case Qt::Key_Down:
            row = qMin(rows - 1, row + 1);
            break;
        case Qt::Key_Left:
            column = qMax(0, column - 1);
            break;
        case Qt::Key_Right:
            column = qMin(columns - 1, column + 1);
            break;
        default:
            QVERIFY(false);
        }

        QApplication::instance()->processEvents();

        QModelIndex index = model.index(row, column);
        QCOMPARE(view.currentIndex().row(), row);
        QCOMPARE(view.currentIndex().column(), column);
        QCOMPARE(view.currentIndex(), index);
    }
}

void tst_QTableView::headerSections_data()
{

}

void tst_QTableView::headerSections()
{
    QtTestTableModel model(10, 10);

    QTableView view;
    QHeaderView *hheader = view.horizontalHeader();
    QHeaderView *vheader = view.verticalHeader();

    view.setModel(&model);
    view.show();

    hheader->doItemsLayout();
    vheader->doItemsLayout();

    QCOMPARE(hheader->count(), model.columnCount());
    QCOMPARE(vheader->count(), model.rowCount());
}

void tst_QTableView::headerSections_unhideRow()
{
    QtTestTableModel model(10, 10);
    QTableView view;

    view.setModel(&model);
    view.setRowHidden(0, true);
    view.show();

    // should go back to old size
    view.setRowHidden(0, false);
    QVERIFY(view.verticalHeader()->sectionSize(0) > 0);

}

void tst_QTableView::moveCursor2()
{
    QtTestTableModel model(10, 10);
    QTableView view;
    
    view.setModel(&model);

    QTest::keyClick(&view, Qt::Key_Down);

    view.setModel(0);
    view.setModel(&model);
    view.setRowHidden(0, true);
    view.setColumnHidden(0, true);

    QTest::keyClick(&view, Qt::Key_Down);
    QCOMPARE(view.selectionModel()->currentIndex(), model.index(1, 1));
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
    QTest::newRow("MoveRight (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0 
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << 0 << 1;

    QTest::newRow("MoveRight (3, 0)")
        << 4 << 4 << -1 << -1
        << 3 << 0
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << 3 << 1;

    QTest::newRow("MoveRight (3, 3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << -1 << -1; // ###

    QTest::newRow("MoveRight, hidden column 1 (0, 0)")
        << 4 << 4 << -1 << 1
        << 0 << 0
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << 0 << 2;

    QTest::newRow("MoveRight, hidden column 3 (0, 2)")
        << 4 << 4 << -1 << 3
        << 0 << 2
        << int(QtTestTableView::MoveRight) << int(Qt::NoModifier)
        << -1 << -1; // ###

    // MoveNext should in addition wrap
    QTest::newRow("MoveNext (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 1;

    QTest::newRow("MoveNext (0, 2)")
        << 4 << 4 << -1 << -1
        << 0 << 2
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 3;

    QTest::newRow("MoveNext, wrap (0, 3)")
        << 4 << 4 << -1 << -1
        << 0 << 3
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 1 << 0;

    QTest::newRow("MoveNext, wrap (3, 3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveNext, hidden column 1 (0, 0)")
        << 4 << 4 << -1 << 1
        << 0 << 0
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 2;

    QTest::newRow("MoveNext, wrap, hidden column 3 (0, 2)")
        << 4 << 4 << -1 << 3
        << 0 << 2
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 1 << 0;

    QTest::newRow("MoveNext, wrap, hidden column 3 (3, 2)")
        << 4 << 4 << -1 << 3
        << 3 << 2
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveNext, wrapy, wrapx, hidden column 3, hidden row 3 (2, 2)")
        << 4 << 4 << 3 << 3
        << 2 << 2
        << int(QtTestTableView::MoveNext) << int(Qt::NoModifier)
        << 0 << 0;

    // MoveLeft
    QTest::newRow("MoveLeft (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << -1 << -1; // ###

    QTest::newRow("MoveLeft (0, 3)")
        << 4 << 4 << -1 << -1
        << 0 << 3
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << 0 << 2;

    QTest::newRow("MoveLeft (1, 0)")
        << 4 << 4 << -1 << -1
        << 1 << 0
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << -1 << -1; // ###

    QTest::newRow("MoveLeft, hidden column 0 (0, 2)")
        << 4 << 4 << -1 << 1
        << 0 << 2
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveLeft, hidden column 0 (0, 1)")
        << 4 << 4 << -1 << 0
        << 0 << 1
        << int(QtTestTableView::MoveLeft) << int(Qt::NoModifier)
        << -1 << -1;

    // MovePrevious should in addition wrap
    QTest::newRow("MovePrevious (0, 3)")
        << 4 << 4 << -1 << -1
        << 0 << 3
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 2;

    QTest::newRow("MovePrevious (0, 1)")
        << 4 << 4 << -1 << -1
        << 0 << 1
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MovePrevious, wrap (1, 0)")
        << 4 << 4 << -1 << -1
        << 1 << 0
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 3;

    QTest::newRow("MovePrevious, wrap, (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 3 << 3;

    QTest::newRow("MovePrevious, hidden column 1 (0, 2)")
        << 4 << 4 << -1 << 1
        << 0 << 2
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MovePrevious, wrap, hidden column 3 (0, 2)")
        << 4 << 4 << -1 << 3
        << 0 << 2
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 0 << 1;

    QTest::newRow("MovePrevious, wrapy, hidden column 0 (0, 1)")
        << 4 << 4 << -1 << 0
        << 0 << 1
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 3 << 3;

    QTest::newRow("MovePrevious, wrap, hidden column 0, hidden row 0 (1, 1)")
        << 4 << 4 << 0 << 0
        << 1 << 1
        << int(QtTestTableView::MovePrevious) << int(Qt::NoModifier)
        << 3 << 3;

    // MoveDown
    QTest::newRow("MoveDown (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << 1 << 0;

    QTest::newRow("MoveDown (3, 0)")
        << 4 << 4 << -1 << -1
        << 3 << 0
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << -1 << -1; // ###

    QTest::newRow("MoveDown (3, 3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << -1 << -1; // ###

    QTest::newRow("MoveDown, hidden row 1 (0, 0)")
        << 4 << 4 << 1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << 2 << 0;

    QTest::newRow("MoveDown, hidden row 3 (2, 0)")
        << 4 << 4 << 3 << -1
        << 2 << 0
        << int(QtTestTableView::MoveDown) << int(Qt::NoModifier)
        << -1 << -1;

    // MoveUp
    QTest::newRow("MoveUp (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << -1 << -1;

    QTest::newRow("MoveUp (3, 0)")
        << 4 << 4 << -1 << -1
        << 3 << 0
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << 2 << 0;

    QTest::newRow("MoveUp (0, 1)")
        << 4 << 4 << -1 << -1
        << 0 << 1
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << -1 << -1;

    QTest::newRow("MoveUp, hidden row 1 (2, 0)")
        << 4 << 4 << 1 << -1
        << 2 << 0
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveUp, hidden row (1, 0)")
        << 4 << 4 << 0 << -1
        << 1 << 0
        << int(QtTestTableView::MoveUp) << int(Qt::NoModifier)
        << -1 << -1;

    // MoveHome
    QTest::newRow("MoveHome (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveHome) << int(Qt::NoModifier)
        << 0 << 0;

    QTest::newRow("MoveHome (3, 3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveHome) << int(Qt::NoModifier)
        << 3 << 0;

    QTest::newRow("MoveHome, hidden column 0 (3, 3)")
        << 4 << 4 << -1 << 0
        << 3 << 3
        << int(QtTestTableView::MoveHome) << int(Qt::NoModifier)
        << 3 << 1;

    // Use Ctrl modifier
    QTest::newRow("MoveHome + Ctrl (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveHome) << int(Qt::ControlModifier)
        << 0 << 0;

    QTest::newRow("MoveHome + Ctrl (3, 3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveHome) << int(Qt::ControlModifier)
        << 0 << 0;

    QTest::newRow("MoveHome + Ctrl, hidden column 0, hidden row 0 (3, 3)")
        << 4 << 4 << 0 << 0
        << 3 << 3
        << int(QtTestTableView::MoveHome) << int(Qt::ControlModifier)
        << 1 << 1;

    // MoveEnd
    QTest::newRow("MoveEnd (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::NoModifier)
        << 0 << 3;

    QTest::newRow("MoveEnd (3, 3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveEnd) << int(Qt::NoModifier)
        << 3 << 3;

    QTest::newRow("MoveEnd, hidden column (0, 0)")
        << 4 << 4 << -1 << 3
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::NoModifier)
        << 0<< 2;

    // Use Ctrl modifier
    QTest::newRow("MoveEnd + Ctrl (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::ControlModifier)
        << 3 << 3;

    QTest::newRow("MoveEnd + Ctrl (3,3)")
        << 4 << 4 << -1 << -1
        << 3 << 3
        << int(QtTestTableView::MoveEnd) << int(Qt::ControlModifier)
        << 3 << 3;

    QTest::newRow("MoveEnd + Ctrl, hidden column 3 (0, 0)")
        << 4 << 4 << -1 << 3
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::ControlModifier)
        << 3 << 2;

    QTest::newRow("MoveEnd + Ctrl, hidden column 3, hidden row 3 (0,0)")
        << 4 << 4 << 3 << 3
        << 0 << 0
        << int(QtTestTableView::MoveEnd) << int(Qt::ControlModifier)
        << 2 << 2;

    QTest::newRow("MovePageUp (0, 0)")
        << 4 << 4 << -1 << -1
        << 0 << 0
        << int(QtTestTableView::MovePageUp) << 0
        << 0 << 0;

    QTest::newRow("MovePageUp (3, 3)")
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

    QCOMPARE(newIndex.row(), expectedRow);
    QCOMPARE(newIndex.column(), expectedColumn);
}

void tst_QTableView::hideRows()
{
    QtTestTableModel model(10, 10);
    QTableView view;

    view.setModel(&model);

    view.setRowHidden(0, true);
    QVERIFY(view.isRowHidden(0));

    view.setRowHidden(3, true);
    QVERIFY(view.isRowHidden(3));

    view.setRowHidden(0, false);
    QVERIFY(!view.isRowHidden(0));
    QVERIFY(view.isRowHidden(3));
}

void tst_QTableView::hideColumns()
{
    QtTestTableModel model(10, 10);

    QTableView view;
    view.setModel(&model);

    view.setColumnHidden(0, true);
    QVERIFY(view.isColumnHidden(0));

    view.setColumnHidden(3, true);
    QVERIFY(view.isColumnHidden(3));

    view.setColumnHidden(0, false);
    QVERIFY(!view.isColumnHidden(0));
    QVERIFY(view.isColumnHidden(3));
}


void tst_QTableView::selectRow_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("behavior");
    QTest::addColumn<bool>("selectionExpected");

    QTest::newRow("SingleSelection and SelectItems")
        << 0
        << (int)QAbstractItemView::SingleSelection
        << (int)QAbstractItemView::SelectItems
        << false;

    QTest::newRow("SingleSelection and SelectRows")
        << 0
        << (int)QAbstractItemView::SingleSelection
        << (int)QAbstractItemView::SelectRows
        << true;

    QTest::newRow("SingleSelection and SelectColumns")
        << 0
        << (int)QAbstractItemView::SingleSelection
        << (int)QAbstractItemView::SelectColumns
        << false;

    QTest::newRow("MultiSelection and SelectItems")
        << 0
        << (int)QAbstractItemView::MultiSelection
        << (int)QAbstractItemView::SelectItems
        << true;

    QTest::newRow("MultiSelection and SelectRows")
        << 0
        << (int)QAbstractItemView::MultiSelection
        << (int)QAbstractItemView::SelectRows
        << true;

    QTest::newRow("MultiSelection and SelectColumns")
        << 0
        << (int)QAbstractItemView::MultiSelection
        << (int)QAbstractItemView::SelectColumns
        << false;

    QTest::newRow("ExtendedSelection and SelectItems")
        << 0
        << (int)QAbstractItemView::ExtendedSelection
        << (int)QAbstractItemView::SelectItems
        << true;

    QTest::newRow("ExtendedSelection and SelectRows")
        << 0
        << (int)QAbstractItemView::ExtendedSelection
        << (int)QAbstractItemView::SelectRows
        << true;

    QTest::newRow("ExtendedSelection and SelectColumns")
        << 0
        << (int)QAbstractItemView::ExtendedSelection
        << (int)QAbstractItemView::SelectColumns
        << false;

    QTest::newRow("ContiguousSelection and SelectItems")
        << 0
        << (int)QAbstractItemView::ContiguousSelection
        << (int)QAbstractItemView::SelectItems
        << true;

    QTest::newRow("ContiguousSelection and SelectRows")
        << 0
        << (int)QAbstractItemView::ContiguousSelection
        << (int)QAbstractItemView::SelectRows
        << true;

    QTest::newRow("ContiguousSelection and SelectColumns")
        << 0
        << (int)QAbstractItemView::ContiguousSelection
        << (int)QAbstractItemView::SelectColumns
        << false;
}

void tst_QTableView::selectRow()
{
    QFETCH(int, row);
    QFETCH(int, mode);
    QFETCH(int, behavior);
    QFETCH(bool, selectionExpected);

    QtTestTableModel model(10, 10);
    QTableView view;

    view.setModel(&model);
    view.setSelectionMode((QAbstractItemView::SelectionMode)mode);
    view.setSelectionBehavior((QAbstractItemView::SelectionBehavior)behavior);

    QCOMPARE(view.selectionModel()->selectedIndexes().count(), 0);

    view.selectRow(row);

    //test we have 10 items selected
    QCOMPARE(view.selectionModel()->selectedIndexes().count(), selectionExpected ? 10 : 0);
    //test that all 10 items are in the same row
    for (int i = 0; selectionExpected && i < 10; ++i)
        QCOMPARE(view.selectionModel()->selectedIndexes().at(i).row(), row);
}

void tst_QTableView::selectColumn_data()
{
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("behavior");
    QTest::addColumn<bool>("selectionExpected");

        QTest::newRow("SingleSelection and SelectItems")
            << 0
            << (int)QAbstractItemView::SingleSelection
            << (int)QAbstractItemView::SelectItems
            << false;

        QTest::newRow("SingleSelection and SelectRows")
            << 0
            << (int)QAbstractItemView::SingleSelection
            << (int)QAbstractItemView::SelectRows
            << false;

        QTest::newRow("SingleSelection and SelectColumns")
            << 0
            << (int)QAbstractItemView::SingleSelection
            << (int)QAbstractItemView::SelectColumns
            << true;

        QTest::newRow("MultiSelection and SelectItems")
            << 0
            << (int)QAbstractItemView::MultiSelection
            << (int)QAbstractItemView::SelectItems
            << true;

        QTest::newRow("MultiSelection and SelectRows")
            << 0
            << (int)QAbstractItemView::MultiSelection
            << (int)QAbstractItemView::SelectRows
            << false;

        QTest::newRow("MultiSelection and SelectColumns")
            << 0
            << (int)QAbstractItemView::MultiSelection
            << (int)QAbstractItemView::SelectColumns
            << true;

        QTest::newRow("ExtendedSelection and SelectItems")
            << 0
            << (int)QAbstractItemView::ExtendedSelection
            << (int)QAbstractItemView::SelectItems
            << true;

        QTest::newRow("ExtendedSelection and SelectRows")
            << 0
            << (int)QAbstractItemView::ExtendedSelection
            << (int)QAbstractItemView::SelectRows
            << false;

        QTest::newRow("ExtendedSelection and SelectColumns")
            << 0
            << (int)QAbstractItemView::ExtendedSelection
            << (int)QAbstractItemView::SelectColumns
            << true;

        QTest::newRow("ContiguousSelection and SelectItems")
            << 0
            << (int)QAbstractItemView::ContiguousSelection
            << (int)QAbstractItemView::SelectItems
            << true;

        QTest::newRow("ContiguousSelection and SelectRows")
            << 0
            << (int)QAbstractItemView::ContiguousSelection
            << (int)QAbstractItemView::SelectRows
            << false;

        QTest::newRow("ContiguousSelection and SelectColumns")
            << 0
            << (int)QAbstractItemView::ContiguousSelection
            << (int)QAbstractItemView::SelectColumns
            << true;
}

void tst_QTableView::selectColumn()
{
    QFETCH(int, column);
    QFETCH(int, mode);
    QFETCH(int, behavior);
    QFETCH(bool, selectionExpected);

    QtTestTableModel model(10, 10);

    QTableView view;
    view.setModel(&model);
    view.setSelectionMode((QAbstractItemView::SelectionMode)mode);
    view.setSelectionBehavior((QAbstractItemView::SelectionBehavior)behavior);

    QCOMPARE(view.selectionModel()->selectedIndexes().count(), 0);

    view.selectColumn(column);

    //test we have 10 items selected
    QCOMPARE(view.selectionModel()->selectedIndexes().count(), selectionExpected ? 10 : 0);
    //test that all 10 items are in the same column
    for (int i = 0; selectionExpected && i < 10; ++i)
        QCOMPARE(view.selectionModel()->selectedIndexes().at(i).column(), column);
}

Q_DECLARE_METATYPE(QRect)
void tst_QTableView::visualRect_data()
{
    QTest::addColumn<int>("hiderow");
    QTest::addColumn<int>("hidecolumn");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("col");
    QTest::addColumn<QRect>("expectedRect");

    // MoveRight
    QTest::newRow("(0,0)") << -1 << -1 << 0 << 0 << QRect(0,0,29,19);
    QTest::newRow("(0,0), hidden row") << 0 << -1 << 0 << 0 << QRect();
    QTest::newRow("(0,0), hidden column") << -1 << 0 << 0 << 0 << QRect();
    QTest::newRow("(0,0), hidden row and column") << 0 << 0 << 0 << 0 << QRect();
    QTest::newRow("(0,0), out of bounds") << -1 << -1 << 20 << 20 << QRect();
    QTest::newRow("(5,5), hidden row") << 5 << -1 << 5 << 5 << QRect();
    QTest::newRow("(9,9)") << -1 << -1 << 9 << 9 << QRect(30*9,20*9,29,19);

}

void tst_QTableView::visualRect()
{
    QFETCH(int, hiderow);
    QFETCH(int, hidecolumn);
    QFETCH(int, row);
    QFETCH(int, col);
    QFETCH(QRect, expectedRect);

    QtTestTableModel model(10, 10);

    QTableView view;
    view.setModel(&model);
    // Make sure that it has 1 pixel between each cell.
    view.setGridStyle(Qt::SolidLine);
    int i;
    for (i = 0; i < view.horizontalHeader()->count(); i++) {
        view.horizontalHeader()->resizeSection(i,30);
    }
    for (i = 0; i < view.verticalHeader()->count(); i++) {
        view.verticalHeader()->resizeSection(i,20);
    }

    if (hiderow >= 0) view.setRowHidden(hiderow, true);
    if (hidecolumn >= 0) view.setColumnHidden(hidecolumn, true);

    QRect rect = view.visualRect(model.index(row,col));
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
}

void tst_QTableView::rowViewportPosition()
{
}
    
void tst_QTableView::rowAt_data()
{
}

void tst_QTableView::rowAt()
{
}

void tst_QTableView::rowHeight_data()
{
}

void tst_QTableView::rowHeight()
{
}

void tst_QTableView::columnViewportPosition_data()
{
}

void tst_QTableView::columnViewportPosiiton()
{
}

void tst_QTableView::columnAt_data()
{
}

void tst_QTableView::columnAt()
{
}

void tst_QTableView::columnWidth_data()
{
}

void tst_QTableView::columnWidth()
{
}

void tst_QTableView::hiddeRow_data()
{
}

void tst_QTableView::hiddeRow()
{
}

void tst_QTableView::hiddeColumn_data()
{
}

void tst_QTableView::hiddnColumn()
{
}

void tst_QTableView::sortingEnabled_data()
{
}

void tst_QTableView::sortingEnabled()
{
}

void tst_QTableView::scrollTo_data()
{
}

void tst_QTableView::scrollTo()
{
}

void tst_QTableView::indexAt_data()
{
}

void tst_QTableView::indexAt()
{
}

void tst_QTableView::rowSpan_data()
{
}

void tst_QTableView::rowSpan()
{
}
    
void tst_QTableView::columnSpan_data()
{
}

void tst_QTableView::columnSpan()
{
}

QTEST_MAIN(tst_QTableView)
#include "tst_qtableview.moc"
