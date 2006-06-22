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
    void removeRows();
    void keyboardNavigation();
    void headerSections();
    void headerSections_unhideRow();
    void moveCursor2();
    void moveCursor_data();
    void moveCursor();
    //void moveCursorDownInOpenEditor();
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
};

// Testing get/set functions
void tst_QTableView::getSetCheck()
{
    QTableView obj1;
    // QHeaderView * QTableView::horizontalHeader()
    // void QTableView::setHorizontalHeader(QHeaderView *)
    QHeaderView *var1 = new QHeaderView(Qt::Horizontal);
    obj1.setHorizontalHeader(var1);
    QCOMPARE(var1, obj1.horizontalHeader());
#if QT_VERSION >= 0x040200
    // Itemviews in Qt < 4.2 have asserts for this. Qt >= 4.2 should handle this gracefully
    obj1.setHorizontalHeader((QHeaderView *)0);
    //QCOMPARE((QHeaderView *)0, obj1.horizontalHeader());
    QCOMPARE(var1, obj1.horizontalHeader());
#endif
    delete var1;

    // QHeaderView * QTableView::verticalHeader()
    // void QTableView::setVerticalHeader(QHeaderView *)
    QHeaderView *var2 = new QHeaderView(Qt::Vertical);
    obj1.setVerticalHeader(var2);
    QCOMPARE(var2, obj1.verticalHeader());
#if QT_VERSION >= 0x040200
    // Itemviews in Qt < 4.2 have asserts for this. Qt >= 4.2 should handle this gracefully
    obj1.setVerticalHeader((QHeaderView *)0);
    //QCOMPARE((QHeaderView *)0, obj1.verticalHeader());
    QCOMPARE(var2, obj1.verticalHeader());
#endif
    delete var2;

    // bool QTableView::showGrid()
    // void QTableView::setShowGrid(bool)
    obj1.setShowGrid(false);
    QCOMPARE(false, obj1.showGrid());
    obj1.setShowGrid(true);
    QCOMPARE(true, obj1.showGrid());
}

class QtTestModel: public QAbstractTableModel
{
public:
    QtTestModel(QObject *parent = 0): QAbstractTableModel(parent),
       cols(0), rows(0), canfetchmore(false), wrongIndex(false), fetchMoreCount(0) {}
    int rowCount(const QModelIndex&) const { return rows; }
    int columnCount(const QModelIndex&) const { return cols; }
    bool isEditable(const QModelIndex &) const { return true; }

    QVariant data(const QModelIndex &idx, int role) const
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            if (idx.row() < 0 || idx.column() < 0 || idx.column() >= cols || idx.row() >= rows) {
                wrongIndex = true;
                qWarning("Invalid modelIndex [%d,%d,%p]", idx.row(), idx.column(), idx.internalPointer());
            }
            return QString("[%1,%2,%3]").arg(idx.row()).arg(idx.column()).arg(0);//idx.data());
        } else {
            return QVariant();
        }
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

    bool canFetchMore(const QModelIndex &) const
    {
        return canfetchmore;
    }

    void fetchMore(const QModelIndex &)
    {
        ++fetchMoreCount;
    }


    int cols, rows;
    bool canfetchmore;
    mutable bool wrongIndex;

    int fetchMoreCount;
};

class MoveCursorTableView : public QTableView {
    public:
    typedef enum {
        MoveUp = 0,
        MoveDown,
        MoveLeft,
        MoveRight,
        MoveHome,
        MoveEnd,
        MovePageUp,
        MovePageDown,
        MoveNext,
        MovePrevious
    } CursorAction;

    QModelIndex moveCursor(MoveCursorTableView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) {
        return QTableView::moveCursor((QAbstractItemView::CursorAction)cursorAction, modifiers);
    }
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
    QtTestModel model;
    QTableView view;
    view.setModel(&model);
    view.show();
    QVERIFY(!model.wrongIndex);
}

void tst_QTableView::removeRows()
{
    QtTestModel model(0);
    model.rows = model.cols = 10;

    QTableView view;
    view.setModel(&model);
    view.show();

    model.removeLastRow();
    QVERIFY(!model.wrongIndex);

    model.removeAllRows();
    QVERIFY(!model.wrongIndex);
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

    int row    = rows - 1;
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

class Dmodel : public QDirModel
{
public:
    int columnCount(const QModelIndex &parent) const
    {
	if (parent == index(QDir::currentPath()))
	    return 1;
	return QDirModel::columnCount(parent);
    }
};

void tst_QTableView::headerSections()
{
    Dmodel model;

    QTableView view;
    QHeaderView *hheader = view.horizontalHeader();
    QHeaderView *vheader = view.verticalHeader();

    view.setModel(&model);
    view.show();

    QModelIndex index = model.index(QDir::currentPath());
    view.setRootIndex(index);
    QApplication::processEvents();
    QCOMPARE(hheader->count(), model.columnCount(index));
    QCOMPARE(vheader->count(), model.rowCount(index));
}

// Originated from task #82987
void tst_QTableView::headerSections_unhideRow()
{
    QtTestModel model;
    model.rows = model.cols = 10;

    QTableView view;

    view.setModel(&model);
    view.setRowHidden(0, true);
    view.show();
    QApplication::processEvents();  // Important. This would issue QHeaderView::doItemsLayout() which then clear the hiddenSectionSize information.

    // should go back to old size
    view.setRowHidden(0, false);
    QVERIFY(view.verticalHeader()->sectionSize(0) > 0);

}

void tst_QTableView::moveCursor2()
{
    QtTestModel model;
    model.rows = model.cols = 10;

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

#if 0
// This test is disabled because the changing of focus is dependent on the window manager,
// so this test will produce different results on different window managers, and may even fail
// randomly on others (metacity being one of them)
// Possibly a candidate for squish testing
void tst_QTableView::moveCursorDownInOpenEditor()
{
    QStandardItemModel model(10, 10, this);
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 10; ++j)
            model.setData(model.index(i,j), QString("[%1, %2]").arg(j).arg(i));

    QTableView view;
    view.setModel(&model);
    QModelIndex idx00 = model.index(0,0);
    view.setCurrentIndex(idx00);
    view.setFocus(Qt::MouseFocusReason);
    view.show();

    QCOMPARE(view.currentIndex(), idx00);
    QTest::keyClick(&view, Qt::Key_F2);
    qApp->processEvents();      // Allow focus to change
    QWidget *fw = qApp->focusWidget();
    QVERIFY(fw); // fw should now be the editor
    // Note that this test fails from time to time for some reason
    // it seems that the focus is not set on the editor fast enough

    QTest::keyClick(fw, Qt::Key_9);
    qApp->processEvents();
    QWidget *editor = view.focusWidget();

    QTest::keyClick(qApp->focusWidget(), Qt::Key_Down);
    qApp->processEvents();
    QVariant val = model.data(idx00, Qt::DisplayRole);
    QCOMPARE(val.toString(), QString("[0, 0]9"));
    QCOMPARE(view.currentIndex(), model.index(1,0));

    QTest::keyClick(qApp->focusWidget(), Qt::Key_Down);
    qApp->processEvents();
    QCOMPARE(view.currentIndex(), model.index(2,0));

    // And finally, the focusWidget should be the view, not the editor.
    QCOMPARE((void*)qApp->focusWidget(), (void*)&view);
}
#endif

void tst_QTableView::moveCursor_data()
{
    QTest::addColumn<int>("startx");
    QTest::addColumn<int>("starty");
    QTest::addColumn<int>("cursorMoveAction");
    QTest::addColumn<int>("modifier");
    QTest::addColumn<int>("resultx");
    QTest::addColumn<int>("resulty");
    QTest::addColumn<int>("hideColumn");
    QTest::addColumn<int>("hideRow");

    // MoveRight
    QTest::newRow("MoveRight (0,0)")
        << 0 << 0
        << int(MoveCursorTableView::MoveRight)
        << int(Qt::NoModifier)
        << 1 << 0
        << -1 << -1;

    QTest::newRow("MoveRight (3,0)")
        << 3 << 0
        << (int)MoveCursorTableView::MoveRight
        << (int)Qt::NoModifier << -1 << -1
        << -1 << -1;

    QTest::newRow("MoveRight (3,3)")
        << 3 << 3
        << int(MoveCursorTableView::MoveRight)
        << int(Qt::NoModifier)
        << -1 << -1
        << -1 << -1;

    QTest::newRow("MoveRight, hidden column (0,0)")
        << 0 << 0
        << int(MoveCursorTableView::MoveRight)
        << int(Qt::NoModifier)
        << 2 << 0
        << 1 << -1;

    QTest::newRow("MoveRight, hidden column (2,0)")
        << 2 << 0
        << int(MoveCursorTableView::MoveRight)
        << int(Qt::NoModifier)
        << -1 << -1
        << 3 << -1;

    // MoveNext should in addition wrap
    QTest::newRow("MoveNext (0,0)")
        << 0 << 0
        << int(MoveCursorTableView::MoveNext)
        << int(Qt::NoModifier)
        << 1 << 0
        << -1 << -1;

    QTest::newRow("MoveNext (2,0)")
        << 2 << 0
        << int(MoveCursorTableView::MoveNext)
        << int(Qt::NoModifier)
        << 3 << 0
        << -1 << -1;

    QTest::newRow("MoveNext, wrapx (3,0)")
        << 3 << 0
        << int(MoveCursorTableView::MoveNext)
        << int(Qt::NoModifier)
        << 0 << 1
        << -1 << -1;

    QTest::newRow("MoveNext, wrapy, wrapx (3,3)")
        << 3 << 3
        << int(MoveCursorTableView::MoveNext)
        << int(Qt::NoModifier)
        << 0 << 0
        << -1 << -1;

    QTest::newRow("MoveNext, hidden column (0,0)")
        << 0 << 0
        << int(MoveCursorTableView::MoveNext)
        << int(Qt::NoModifier)
        << 2 << 0
        << 1 << -1;

    QTest::newRow("MoveNext, wrapx, hidden column (2,0)")
        << 2 << 0
        << int(MoveCursorTableView::MoveNext)
        << int(Qt::NoModifier)
        << 0 << 1
        << 3 << -1;

    QTest::newRow("MoveNext, wrapy, wrapx, hidden column (2,3)")
        << 2 << 3
        << int(MoveCursorTableView::MoveNext)
        << int(Qt::NoModifier)
        << 0 << 0
        << 3 << -1;

    QTest::newRow("MoveNext, wrapy, wrapx, hidden column, hidden row (2,2)")
        << 2 << 2
        << int(MoveCursorTableView::MoveNext)
        << int(Qt::NoModifier)
        << 0 << 0
        << 3 << 3;

    // MoveLeft
    QTest::newRow("MoveLeft (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveLeft
        << (int)Qt::NoModifier
        << -1 << -1
        << -1 << -1;

    QTest::newRow("MoveLeft (3,0)")
        << 3 << 0
        << (int)MoveCursorTableView::MoveLeft
        << (int)Qt::NoModifier
        << 2 << 0
        << -1 << -1;

    QTest::newRow("MoveLeft (0,1)")
        << 0 << 1
        << (int)MoveCursorTableView::MoveLeft
        << (int)Qt::NoModifier
        << -1 << -1
        << -1 << -1;

    QTest::newRow("MoveLeft, hidden column (2,0)")
        << 2 << 0
        << (int)MoveCursorTableView::MoveLeft
        << (int)Qt::NoModifier
        << 0 << 0
        << 1 << -1;

    QTest::newRow("MoveLeft, hidden column (1,0)")
        << 1 << 0
        << (int)MoveCursorTableView::MoveLeft
        << (int)Qt::NoModifier
        << -1 << -1
        << 0 << -1;

    // MovePrevious should in addition wrap
    QTest::newRow("MovePrevious (3,0)")
        << 3 << 0
        << (int)MoveCursorTableView::MovePrevious
        << (int)Qt::NoModifier
        << 2 << 0
        << -1 << -1;

    QTest::newRow("MovePrevious (1,0)")
        << 1 << 0
        << (int)MoveCursorTableView::MovePrevious
        << (int)Qt::NoModifier
        << 0 << 0
        << -1 << -1;

    QTest::newRow("MovePrevious, wrapx (0,1)")
        << 0 << 1
        << (int)MoveCursorTableView::MovePrevious
        << (int)Qt::NoModifier
        << 3 << 0
        << -1 << -1;

    QTest::newRow("MovePrevious, wrapy, wrapx (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MovePrevious
        << (int)Qt::NoModifier
        << 3 << 3
        << -1 << -1;

    QTest::newRow("MovePrevious, hidden column (2,0)")
        << 2 << 0
        << (int)MoveCursorTableView::MovePrevious
        << (int)Qt::NoModifier
        << 0 << 0
        << 1 << -1;

    QTest::newRow("MovePrevious, wrapx, hidden column (0,1)")
        << 0 << 1
        << (int)MoveCursorTableView::MovePrevious
        << (int)Qt::NoModifier
        << 2 << 0
        << 3 << -1;

    QTest::newRow("MovePrevious, wrapy, wrapx, hidden column (1,0)")
        << 1 << 0
        << (int)MoveCursorTableView::MovePrevious
        << (int)Qt::NoModifier
        << 3 << 3
        << 0 << -1;

    QTest::newRow("MovePrevious, wrapy, wrapx, hidden column, hidden row (1,1)")
        << 1 << 1
        << (int)MoveCursorTableView::MovePrevious
        << (int)Qt::NoModifier
        << 3 << 3
        << 0 << 0;

    // MoveDown
    QTest::newRow("MoveDown (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveDown
        << (int)Qt::NoModifier
        << 0 << 1
        << -1 << -1;

    QTest::newRow("MoveDown (0,3)")
        << 0 << 3
        << (int)MoveCursorTableView::MoveDown
        << (int)Qt::NoModifier
        << -1 << -1
        << -1 << -1;

    QTest::newRow("MoveDown (3,3)")
        << 3 << 3
        << (int)MoveCursorTableView::MoveDown
        << (int)Qt::NoModifier
        << -1 << -1
        << -1 << -1;

    QTest::newRow("MoveDown, hidden row (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveDown
        << (int)Qt::NoModifier
        << 0 << 2
        << -1 << 1;

    QTest::newRow("MoveDown, hidden row (0,2)")
        << 0 << 2
        << (int)MoveCursorTableView::MoveDown
        << (int)Qt::NoModifier
        << -1 << -1
        << -1 << 3;

    // MoveUp
    QTest::newRow("MoveUp (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveUp
        << (int)Qt::NoModifier
        << -1 << -1
        << -1 << -1;

    QTest::newRow("MoveUp (0,3)")
        << 0 << 3
        << (int)MoveCursorTableView::MoveUp
        << (int)Qt::NoModifier
        << 0 << 2
        << -1 << -1;

    QTest::newRow("MoveUp (1,0)")
        << 1 << 0
        << (int)MoveCursorTableView::MoveUp
        << (int)Qt::NoModifier
        << -1 << -1
        << -1 << -1;

    QTest::newRow("MoveUp, hidden row (0,2)")
        << 0 << 2
        << (int)MoveCursorTableView::MoveUp
        << (int)Qt::NoModifier
        << 0 << 0
        << -1 << 1;

    QTest::newRow("MoveUp, hidden row (0,1)")
        << 0 << 1
        << (int)MoveCursorTableView::MoveUp
        << (int)Qt::NoModifier
        << -1 << -1
        << -1 << 0;

    // MoveHome
    QTest::newRow("MoveHome (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveHome
        << (int)Qt::NoModifier
        << 0 << 0
        << -1 << -1;

    QTest::newRow("MoveHome (3,3)")
        << 3 << 3
        << (int)MoveCursorTableView::MoveHome
        << (int)Qt::NoModifier
        << 0 << 3
        << -1 << -1;

    QTest::newRow("MoveHome, hidden column (3,3)")
        << 3 << 3
        << (int)MoveCursorTableView::MoveHome
        << (int)Qt::NoModifier
        << 1 << 3
        << 0 << -1;

    // Use Ctrl modifier
    QTest::newRow("MoveHome + Ctrl (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveHome
        << (int)Qt::ControlModifier
        << 0 << 0
        << -1 << -1;

    QTest::newRow("MoveHome + Ctrl (3,3)")
        << 3 << 3
        << (int)MoveCursorTableView::MoveHome
        << (int)Qt::ControlModifier
        << 0 << 0
        << -1 << -1;

    QTest::newRow("MoveHome + Ctrl, hidden column, hidden row (3,3)")
        << 3 << 3
        << (int)MoveCursorTableView::MoveHome
        << (int)Qt::ControlModifier
        << 1 << 1
        << 0 << 0;

    // MoveEnd
    QTest::newRow("MoveEnd (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveEnd
        << (int)Qt::NoModifier
        << 3 << 0
        << -1 << -1;

    QTest::newRow("MoveEnd (3,3)")
        << 3 << 3
        << (int)MoveCursorTableView::MoveEnd
        << (int)Qt::NoModifier
        << 3 << 3
        << -1 << -1;

    QTest::newRow("MoveEnd, hidden column (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveEnd
        << (int)Qt::NoModifier
        << 2 << 0
        << 3 << -1;

    // Use Ctrl modifier
    QTest::newRow("MoveEnd + Ctrl (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveEnd
        << (int)Qt::ControlModifier
        << 3 << 3
        << -1 << -1;

    QTest::newRow("MoveEnd + Ctrl (3,3)")
        << 3 << 3
        << (int)MoveCursorTableView::MoveEnd
        << (int)Qt::ControlModifier
        << 3 << 3
        << -1 << -1;

    QTest::newRow("MoveEnd + Ctrl, hidden column (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveEnd
        << (int)Qt::ControlModifier
        << 2 << 3
        << 3 << -1;

    QTest::newRow("MoveEnd + Ctrl, hidden column, hidden row (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MoveEnd
        << (int)Qt::ControlModifier
        << 2 << 2
        << 3 << 3;

    QTest::newRow("MovePageUp (0,0)")
        << 0 << 0
        << (int)MoveCursorTableView::MovePageUp
        << 0
        << 0 << 0
        << -1 << -1;

    QTest::newRow("MovePageUp (3,3)")
        << 3 << 3
        << (int)MoveCursorTableView::MovePageUp
        << 0
        << 3 << 0
        << -1 << -1;

#if QT_VERSION > 0x040100
    QTest::newRow("MovePageDown (3,3)")
        << 3 << 3
        << (int)MoveCursorTableView::MovePageDown
        << 0
        << 3 << 3
        << -1 << -1;

    QTest::newRow("MovePageDown (3,0)")
        << 3 << 0
        << (int)MoveCursorTableView::MovePageDown
        << 0
        << 3 << 3
        << -1 << -1;
#endif // QT_VERSION
}

void tst_QTableView::moveCursor()
{
    QtTestModel model;
    model.rows = model.cols = 4;

    QFETCH(int, startx);
    QFETCH(int, starty);
    QFETCH(int, cursorMoveAction);
    QFETCH(int, modifier);
    QFETCH(int, resultx);
    QFETCH(int, resulty);
    QFETCH(int, hideColumn);
    QFETCH(int, hideRow);

    MoveCursorTableView view1;
    view1.setModel(&model);
    if (hideColumn != -1) view1.setColumnHidden(hideColumn, true);
    if (hideRow != -1) view1.setRowHidden(hideRow, true);
    view1.show();

    QModelIndex index = model.index(starty,startx);
    view1.setCurrentIndex(index);
    QModelIndex newIndex = view1.moveCursor((MoveCursorTableView::CursorAction)cursorMoveAction, (Qt::KeyboardModifiers)modifier);
    QCOMPARE(newIndex.column(), resultx);
    QCOMPARE(newIndex.row(), resulty);
}

void tst_QTableView::hideRows()
{
    QtTestModel model;
    model.rows = model.cols = 10;

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
    QtTestModel model;
    model.rows = model.cols = 10;

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

    QtTestModel model;
    model.rows = model.cols = 10;

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

    QtTestModel model;
    model.rows = model.cols = 10;

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

    QtTestModel model;
    model.rows = model.cols = 10;

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
    QtTestModel model;
    model.rows = model.cols = 64;
    model.canfetchmore = true;

    QTableView view;
    view.setModel(&model);
    view.show();

    QCOMPARE(model.fetchMoreCount, 0);
    view.verticalScrollBar()->setValue(view.verticalScrollBar()->maximum());
    QVERIFY(model.fetchMoreCount > 0);

    model.fetchMoreCount = 0; //reset
    view.scrollToTop();
    QCOMPARE(model.fetchMoreCount, 0);

    view.scrollToBottom();
    QVERIFY(model.fetchMoreCount > 0);

    model.fetchMoreCount = 0; //reset
    view.scrollToTop();
    view.setCurrentIndex(model.index(0, 0));
    QCOMPARE(model.fetchMoreCount, 0);

    for (int i = 0; i < 64; ++i)
        QTest::keyClick(&view, Qt::Key_Down);
    QCOMPARE(view.currentIndex(), model.index(63, 0));
    QVERIFY(model.fetchMoreCount > 0);
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

QTEST_MAIN(tst_QTableView)
#include "tst_qtableview.moc"
