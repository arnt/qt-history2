/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include <qabstractitemview.h>

#include <QtTest/QtTest>
#include <QtGui/QtGui>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qtreeview.h gui/itemviews/qtreeview.cpp

Q_DECLARE_METATYPE(QModelIndex)

struct PublicView : public QTreeView
{
    inline void executeDelayedItemsLayout()
    { QTreeView::executeDelayedItemsLayout(); }

    enum PublicCursorAction {
        MoveUp = QAbstractItemView::MoveUp,
        MoveDown = QAbstractItemView::MoveDown,
        MoveLeft = QAbstractItemView::MoveLeft,
        MoveRight = QAbstractItemView::MoveRight,
        MoveHome = QAbstractItemView::MoveHome,
        MoveEnd = QAbstractItemView::MoveEnd,
        MovePageUp = QAbstractItemView::MovePageUp,
        MovePageDown = QAbstractItemView::MovePageDown,
        MoveNext = QAbstractItemView::MoveNext,
        MovePrevious = QAbstractItemView::MovePrevious
    };

    inline QModelIndex moveCursor(PublicCursorAction ca, Qt::KeyboardModifiers kbm)
    { return QTreeView::moveCursor((CursorAction)ca, kbm); }
};

class tst_QTreeView : public QObject
{
    Q_OBJECT

public:
    tst_QTreeView();
    virtual ~tst_QTreeView();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void setHeader();
    void setModel();
    void columnHidden();
    void rowHidden();
    void noDelegate();
    void noModel();
    void emptyModel();
    void removeRows();
    void removeCols();
    void expandAndCollapse();
    void expandAndCollapseAll();
    void keyboardNavigation();
    void headerSections();
    void moveCursor();
    void setSelection();
    void indexAbove();
    void indexBelow();
    void indexAt();
    void clicked();
    void mouseDoubleClick();
    void rowsAboutToBeRemoved();
    void headerSections_unhideSection();
    void columnAt();
    void scrollTo();
    void rowsAboutToBeRemoved_move();
    void resizeColumnToContents();
    void insertAfterSelect();
    void removeAfterSelect();
    void hiddenItems();
};

class QtTestModel: public QAbstractItemModel
{
public:
    QtTestModel(QObject *parent = 0): QAbstractItemModel(parent),
       rows(0), cols(0), levels(INT_MAX), wrongIndex(false) {}

    QtTestModel(int _rows, int _cols, QObject *parent = 0): QAbstractItemModel(parent),
       rows(_rows), cols(_cols), levels(INT_MAX), wrongIndex(false) {}

    inline qint32 level(const QModelIndex &index) const {
        return index.isValid() ? qint32(index.internalId()) : qint32(-1);
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const {
        if ((parent.column() > 0) || (level(parent) > levels))
            return 0;
        return rows;
    }
    int columnCount(const QModelIndex& parent = QModelIndex()) const {
        if ((parent.column() > 0) || (level(parent) > levels))
            return 0;
        return cols;
    }

    bool isEditable(const QModelIndex &index) const {
        if (index.isValid())
            return true;
        return false;
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const
    {
        if (row < 0 || column < 0 || (level(parent) > levels) || column >= cols || row >= rows) {
            return QModelIndex();
        }
        QModelIndex i = createIndex(row, column, level(parent) + 1);
	parentHash[i] = parent;
	return i;
    }

    QModelIndex parent(const QModelIndex &index) const
    {
	if (!parentHash.contains(index))
	    return QModelIndex();
	return parentHash[index];
    }

    QVariant data(const QModelIndex &idx, int role) const
    {
        if (!idx.isValid() || role != Qt::DisplayRole)
            return QVariant();

        if (idx.row() < 0 || idx.column() < 0 || idx.column() >= cols || idx.row() >= rows) {
            wrongIndex = true;
            qWarning("Invalid modelIndex [%d,%d,%p]", idx.row(), idx.column(),
		     idx.internalPointer());
        }
        return QString("[%1,%2,%3]").arg(idx.row()).arg(idx.column()).arg(level(idx));
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

    void insertNewRow()
    {
        beginInsertRows(QModelIndex(), 1, rows - 1);
        ++rows;
        endInsertRows();
    }

    int rows, cols;
    int levels;
    mutable bool wrongIndex;
    mutable QMap<QModelIndex,QModelIndex> parentHash;
};

tst_QTreeView::tst_QTreeView()
{
}

tst_QTreeView::~tst_QTreeView()
{
}

void tst_QTreeView::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

void tst_QTreeView::cleanupTestCase()
{
}

void tst_QTreeView::init()
{
}

void tst_QTreeView::cleanup()
{
}

// Testing get/set functions
void tst_QTreeView::getSetCheck()
{
    QTreeView obj1;

    // int QTreeView::indentation()
    // void QTreeView::setIndentation(int)
    QCOMPARE(obj1.indentation(), 20);
    obj1.setIndentation(0);
    QCOMPARE(obj1.indentation(), 0);
    obj1.setIndentation(INT_MIN);
    QCOMPARE(obj1.indentation(), INT_MIN);
    obj1.setIndentation(INT_MAX);
    QCOMPARE(obj1.indentation(), INT_MAX);

    // bool QTreeView::rootIsDecorated()
    // void QTreeView::setRootIsDecorated(bool)
    QCOMPARE(obj1.rootIsDecorated(), true);
    obj1.setRootIsDecorated(false);
    QCOMPARE(obj1.rootIsDecorated(), false);
    obj1.setRootIsDecorated(true);
    QCOMPARE(obj1.rootIsDecorated(), true);

    // bool QTreeView::uniformRowHeights()
    // void QTreeView::setUniformRowHeights(bool)
    QCOMPARE(obj1.uniformRowHeights(), false);
    obj1.setUniformRowHeights(false);
    QCOMPARE(obj1.uniformRowHeights(), false);
    obj1.setUniformRowHeights(true);
    QCOMPARE(obj1.uniformRowHeights(), true);

    // bool QTreeView::itemsExpandable()
    // void QTreeView::setItemsExpandable(bool)
    QCOMPARE(obj1.itemsExpandable(), true);
    obj1.setItemsExpandable(false);
    QCOMPARE(obj1.itemsExpandable(), false);
    obj1.setItemsExpandable(true);
    QCOMPARE(obj1.itemsExpandable(), true);

    // bool QTreeView::allColumnsShowFocus
    // void QTreeView::setAllColumnsShowFocus
    QCOMPARE(obj1.allColumnsShowFocus(), false);
    obj1.setAllColumnsShowFocus(false);
    QCOMPARE(obj1.allColumnsShowFocus(), false);
    obj1.setAllColumnsShowFocus(true);
    QCOMPARE(obj1.allColumnsShowFocus(), true);

    // bool QTreeView::isAnimated
    // void QTreeView::setAnimated
    QCOMPARE(obj1.isAnimated(), false);
    obj1.setAnimated(false);
    QCOMPARE(obj1.isAnimated(), false);
    obj1.setAnimated(true);
    QCOMPARE(obj1.isAnimated(), true);

    // bool QTreeView::setSortingEnabled
    // void QTreeView::isSortingEnabled
    QCOMPARE(obj1.isSortingEnabled(), false);
    obj1.setSortingEnabled(false);
    QCOMPARE(obj1.isSortingEnabled(), false);
    obj1.setSortingEnabled(true);
    QCOMPARE(obj1.isSortingEnabled(), true);
}

void tst_QTreeView::setHeader()
{
    QTreeView view;
    QVERIFY(view.header() != 0);
    QCOMPARE(view.header()->orientation(), Qt::Horizontal);
    QCOMPARE(view.header()->parent(), (QObject *)&view);
    for (int x = 0; x < 2; ++x) {
        QSignalSpy destroyedSpy(view.header(), SIGNAL(destroyed()));
        Qt::Orientation orient = x ? Qt::Vertical : Qt::Horizontal;
        QHeaderView *head = new QHeaderView(orient);
        view.setHeader(head);
        QCOMPARE(destroyedSpy.count(), 1);
        QCOMPARE(head->parent(), (QObject *)&view);
        QCOMPARE(view.header(), head);
        view.setHeader(head);
        QCOMPARE(view.header(), head);
        // Itemviews in Qt < 4.2 have asserts for this. Qt >= 4.2 should handle this gracefully
        view.setHeader((QHeaderView *)0);
        QCOMPARE(view.header(), head);
    }
}

void tst_QTreeView::setModel()
{
    QTreeView view;
    QCOMPARE(view.model(), (QAbstractItemModel*)0);
    QCOMPARE(view.selectionModel(), (QItemSelectionModel*)0);
    QCOMPARE(view.header()->model(), (QAbstractItemModel*)0);
    QCOMPARE(view.header()->selectionModel(), (QItemSelectionModel*)0);
    view.show();
    for (int x = 0; x < 4; ++x) {
        QtTestModel *model = new QtTestModel(10, 8);
        QAbstractItemModel *oldModel = view.model();
        QSignalSpy modelDestroyedSpy(oldModel ? oldModel : model, SIGNAL(destroyed()));
        // set the same model twice
        for (int i = 0; i < 2; ++i) {
            QItemSelectionModel *oldSelectionModel = view.selectionModel();
            QItemSelectionModel *dummy = new QItemSelectionModel(model);
            QSignalSpy selectionModelDestroyedSpy(
                oldSelectionModel ? oldSelectionModel : dummy, SIGNAL(destroyed()));
            view.setModel(model);
//                QCOMPARE(selectionModelDestroyedSpy.count(), (x == 0 || i == 1) ? 0 : 1);
            QCOMPARE(view.model(), (QAbstractItemModel *)model);
            QCOMPARE(view.header()->model(), (QAbstractItemModel *)model);
            QCOMPARE(view.selectionModel() != oldSelectionModel, (i == 0));
            view.update();
            QApplication::processEvents();
        }
        QCOMPARE(modelDestroyedSpy.count(), 0);

        view.setModel(0);
        QCOMPARE(view.model(), (QAbstractItemModel*)0);
        // ### shouldn't selectionModel also be 0 now?
//        QCOMPARE(view.selectionModel(), (QItemSelectionModel*)0);
        view.update();
        QApplication::processEvents();
        delete model;
    }
}

void tst_QTreeView::columnHidden()
{
    QTreeView view;
    QtTestModel model(10, 8);
    view.setModel(&model);
    view.show();
    for (int c = 0; c < model.columnCount(); ++c)
        QCOMPARE(view.isColumnHidden(c), false);
    // hide even columns
    for (int c = 0; c < model.columnCount(); c+=2)
        view.setColumnHidden(c, true);
    for (int c = 0; c < model.columnCount(); ++c)
        QCOMPARE(view.isColumnHidden(c), (c & 1) == 0);
    view.update();
    QApplication::processEvents();
    // hide odd columns too
    for (int c = 1; c < model.columnCount(); c+=2)
        view.setColumnHidden(c, true);
    for (int c = 0; c < model.columnCount(); ++c)
        QCOMPARE(view.isColumnHidden(c), true);
    view.update();
    QApplication::processEvents();
}

void tst_QTreeView::rowHidden()
{
    QtTestModel model(4, 6);
    model.levels = 3;
    QTreeView view;
    view.resize(500,500);
    view.setModel(&model);
    view.show();

    QCOMPARE(view.isRowHidden(-1, QModelIndex()), false);
    QCOMPARE(view.isRowHidden(999999, QModelIndex()), false);
    view.setRowHidden(-1, QModelIndex(), true);
    view.setRowHidden(999999, QModelIndex(), true);
    QCOMPARE(view.isRowHidden(-1, QModelIndex()), false);
    QCOMPARE(view.isRowHidden(999999, QModelIndex()), false);

    view.setRowHidden(0, QModelIndex(), true);
    QCOMPARE(view.isRowHidden(0, QModelIndex()), true);
    view.setRowHidden(0, QModelIndex(), false);
    QCOMPARE(view.isRowHidden(0, QModelIndex()), false);

    QStack<QModelIndex> parents;
    parents.push(QModelIndex());
    while (!parents.isEmpty()) {
        QModelIndex p = parents.pop();
        int rows = model.rowCount(p);
        // hide all
        for (int r = 0; r < rows; ++r) {
            view.setRowHidden(r, p, true);
            QCOMPARE(view.isRowHidden(r, p), true);
        }
        // hide none
        for (int r = 0; r < rows; ++r) {
            view.setRowHidden(r, p, false);
            QCOMPARE(view.isRowHidden(r, p), false);
        }
        // hide only even rows
        for (int r = 0; r < rows; ++r) {
            bool hide = (r & 1) == 0;
            view.setRowHidden(r, p, hide);
            QCOMPARE(view.isRowHidden(r, p), hide);
        }
        for (int r = 0; r < rows; ++r)
            parents.push(model.index(r, 0, p));
    }

    parents.push(QModelIndex());
    while (!parents.isEmpty()) {
        QModelIndex p = parents.pop();
        // all even rows should still be hidden
        for (int r = 0; r < model.rowCount(p); ++r)
            QCOMPARE(view.isRowHidden(r, p), (r & 1) == 0);
        if (model.rowCount(p) > 0) {
            for (int r = 0; r < model.rowCount(p); ++r)
                parents.push(model.index(r, 0, p));
        }
    }
}

void tst_QTreeView::noDelegate()
{
    QtTestModel model(10, 7);
    QTreeView view;
    view.setModel(&model);
    view.setItemDelegate(0);
    QCOMPARE(view.itemDelegate(), (QAbstractItemDelegate *)0);
    view.show();
    QApplication::processEvents();
}

void tst_QTreeView::noModel()
{
    QTreeView view;
    view.show();
    view.setRowHidden(0, QModelIndex(), true);
    QApplication::processEvents();
}

void tst_QTreeView::emptyModel()
{
    QtTestModel model;
    QTreeView view;
    view.setModel(&model);
    view.show();
    QVERIFY(!model.wrongIndex);
}

void tst_QTreeView::removeRows()
{
    QtTestModel model(7, 10);

    QTreeView view;

    view.setModel(&model);
    view.show();

    model.removeLastRow();
    QVERIFY(!model.wrongIndex);

    model.removeAllRows();
    QVERIFY(!model.wrongIndex);
}

void tst_QTreeView::removeCols()
{
    QtTestModel model(5, 8);

    QTreeView view;
    view.setModel(&model);
    view.show();

    model.removeLastColumn();
    QVERIFY(!model.wrongIndex);
    QCOMPARE(view.header()->count(), model.cols);

    model.removeAllColumns();
    QVERIFY(!model.wrongIndex);
    QCOMPARE(view.header()->count(), model.cols);
}

void tst_QTreeView::expandAndCollapse()
{
    QtTestModel model(10, 9);

    QTreeView view;
    view.setModel(&model);
    view.show();

    QModelIndex a = model.index(0, 0, QModelIndex());
    QModelIndex b = model.index(0, 0, a);

    QSignalSpy expandedSpy(&view, SIGNAL(expanded(const QModelIndex&)));
    QSignalSpy collapsedSpy(&view, SIGNAL(collapsed(const QModelIndex&)));
    QVariantList args;

    for (int y = 0; y < 2; ++y) {
        view.setVisible(y == 0);
        for (int x = 0; x < 2; ++x) {
            view.setItemsExpandable(x == 0);

            // Test bad args
            view.expand(QModelIndex());
            QCOMPARE(view.isExpanded(QModelIndex()), false);
            view.collapse(QModelIndex());
            QCOMPARE(expandedSpy.count(), 0);
            QCOMPARE(collapsedSpy.count(), 0);

            // expand a first level item
            QVERIFY(!view.isExpanded(a));
            view.expand(a);
            QVERIFY(view.isExpanded(a));
            QCOMPARE(expandedSpy.count(), 1);
            QCOMPARE(collapsedSpy.count(), 0);
            args = expandedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), a);

            view.expand(a);
            QVERIFY(view.isExpanded(a));
            QCOMPARE(expandedSpy.count(), 0);
            QCOMPARE(collapsedSpy.count(), 0);

            // expand a second level item
            QVERIFY(!view.isExpanded(b));
            view.expand(b);
            QVERIFY(view.isExpanded(a));
            QVERIFY(view.isExpanded(b));
            QCOMPARE(expandedSpy.count(), 1);
            QCOMPARE(collapsedSpy.count(), 0);
            args = expandedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), b);

            view.expand(b);
            QVERIFY(view.isExpanded(b));
            QCOMPARE(expandedSpy.count(), 0);
            QCOMPARE(collapsedSpy.count(), 0);

            // collapse the first level item
            view.collapse(a);
            QVERIFY(!view.isExpanded(a));
            QVERIFY(view.isExpanded(b));
            QCOMPARE(expandedSpy.count(), 0);
            QCOMPARE(collapsedSpy.count(), 1);
            args = collapsedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), a);

            view.collapse(a);
            QVERIFY(!view.isExpanded(a));
            QCOMPARE(expandedSpy.count(), 0);
            QCOMPARE(collapsedSpy.count(), 0);

            // expand the first level item again
            view.expand(a);
            QVERIFY(view.isExpanded(a));
            QVERIFY(view.isExpanded(b));
            QCOMPARE(expandedSpy.count(), 1);
            QCOMPARE(collapsedSpy.count(), 0);
            args = expandedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), a);

            // collapse the second level item
            view.collapse(b);
            QVERIFY(view.isExpanded(a));
            QVERIFY(!view.isExpanded(b));
            QCOMPARE(expandedSpy.count(), 0);
            QCOMPARE(collapsedSpy.count(), 1);
            args = collapsedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), b);

            // collapse the first level item
            view.collapse(a);
            QVERIFY(!view.isExpanded(a));
            QVERIFY(!view.isExpanded(b));
            QCOMPARE(expandedSpy.count(), 0);
            QCOMPARE(collapsedSpy.count(), 1);
            args = collapsedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), a);

            // expand and remove row
            QPersistentModelIndex c = model.index(9, 0, b);
            view.expand(a);
            view.expand(b);
            model.removeLastRow(); // remove c
            QVERIFY(view.isExpanded(a));
            QVERIFY(view.isExpanded(b));
            QVERIFY(!view.isExpanded(c));
            QCOMPARE(expandedSpy.count(), 2);
            QCOMPARE(collapsedSpy.count(), 0);
            args = expandedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), a);
            args = expandedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), b);

            view.collapse(a);
            view.collapse(b);
            QVERIFY(!view.isExpanded(a));
            QVERIFY(!view.isExpanded(b));
            QVERIFY(!view.isExpanded(c));
            QCOMPARE(expandedSpy.count(), 0);
            QCOMPARE(collapsedSpy.count(), 2);
            args = collapsedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), a);
            args = collapsedSpy.takeFirst();
            QCOMPARE(qvariant_cast<QModelIndex>(args.at(0)), b);
        }
    }
}

void tst_QTreeView::expandAndCollapseAll()
{
    QtTestModel model(6, 5);
    model.levels = 2;
    QTreeView view;
    view.setModel(&model);

    QSignalSpy expandedSpy(&view, SIGNAL(expanded(const QModelIndex&)));
    QSignalSpy collapsedSpy(&view, SIGNAL(collapsed(const QModelIndex&)));

    view.expandAll();
    view.show();
    QApplication::processEvents();

    QCOMPARE(collapsedSpy.count(), 0);

    QStack<QModelIndex> parents;
    parents.push(QModelIndex());
    int count = 0;
    while (!parents.isEmpty()) {
        QModelIndex p = parents.pop();
        int rows = model.rowCount(p);
        for (int r = 0; r < rows; ++r)
            QVERIFY(view.isExpanded(model.index(r, 0, p)));
        count += rows;
        for (int r = 0; r < rows; ++r)
            parents.push(model.index(r, 0, p));
    }
// ### why is expanded() signal not emitted?
//    QCOMPARE(expandedSpy.count(), count);

    view.collapseAll();

    QCOMPARE(expandedSpy.count(), 0);

    parents.push(QModelIndex());
    count = 0;
    while (!parents.isEmpty()) {
        QModelIndex p = parents.pop();
        int rows = model.rowCount(p);
        for (int r = 0; r < rows; ++r)
            QVERIFY(!view.isExpanded(model.index(r, 0, p)));
        count += rows;
        for (int r = 0; r < rows; ++r)
            parents.push(model.index(r, 0, p));
    }
// ### why is collapsed() signal not emitted?
//    QCOMPARE(collapsedSpy.count(), count);
}

void tst_QTreeView::keyboardNavigation()
{
    const int rows = 10;
    const int columns = 7;

    QtTestModel model(rows, columns);

    QTreeView view;
    view.setModel(&model);
    view.show();

    QVector<Qt::Key> keymoves;
    keymoves << Qt::Key_Down << Qt::Key_Right << Qt::Key_Right << Qt::Key_Right
	     << Qt::Key_Down << Qt::Key_Down << Qt::Key_Down << Qt::Key_Down
	     << Qt::Key_Right << Qt::Key_Right << Qt::Key_Right
	     << Qt::Key_Left << Qt::Key_Up << Qt::Key_Left << Qt::Key_Left
	     << Qt::Key_Up << Qt::Key_Down << Qt::Key_Up << Qt::Key_Up
	     << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up << Qt::Key_Up
	     << Qt::Key_Left << Qt::Key_Left << Qt::Key_Up << Qt::Key_Down;

    int row    = 0;
    int column = 0;
    QModelIndex index = model.index(row, column, QModelIndex());
    view.setCurrentIndex(index);
    QCOMPARE(view.currentIndex(), index);
    QApplication::instance()->processEvents();

    for (int i = 0; i < keymoves.size(); ++i) {
        Qt::Key key = keymoves.at(i);
        QTest::keyClick(&view, key);
        QApplication::instance()->processEvents();

        switch (key) {
        case Qt::Key_Up:
	    if (row > 0) {
		index = index.sibling(row - 1, column);
		row -= 1;
	    } else if (index.parent() != QModelIndex()) {
		index = index.parent();
		row = index.row();
	    }
            break;
        case Qt::Key_Down:
	    if (view.isExpanded(index)) {
		row = 0;
		index = index.child(row, column);
	    } else {
		row = qMin(rows - 1, row + 1);
		index = index.sibling(row, column);
	    }
            break;
        case Qt::Key_Left: {
            QScrollBar *b = view.horizontalScrollBar();
            if (b->value() == b->minimum())
	        QVERIFY(!view.isExpanded(index));
            break;
        }
        case Qt::Key_Right:
	    QVERIFY(view.isExpanded(index));
	    break;
        default:
            QVERIFY(false);
        }

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

void tst_QTreeView::headerSections()
{
    Dmodel model;

    QTreeView view;
    QHeaderView *header = view.header();

    view.setModel(&model);
    view.show();

    QModelIndex index = model.index(QDir::currentPath());
    view.setRootIndex(index);
    QApplication::processEvents();
    QCOMPARE(header->count(), model.columnCount(index));
}

void tst_QTreeView::moveCursor()
{
    QtTestModel model(8, 6);

    PublicView view;
    view.setModel(&model);
    view.setRowHidden(0, QModelIndex(), true);
    view.setColumnHidden(0, true);
    view.show();

    QModelIndex actual = view.moveCursor(PublicView::MoveDown, Qt::NoModifier);
    QModelIndex expected = model.index(1, 1, QModelIndex());

    QCOMPARE(actual, expected);
}

void tst_QTreeView::setSelection()
{
    // ### TODO: implement me
}

void tst_QTreeView::indexAbove()
{
    QtTestModel model(6, 7);
    model.levels = 2;
    QTreeView view;

    QCOMPARE(view.indexAbove(QModelIndex()), QModelIndex());
    view.setModel(&model);
    QCOMPARE(view.indexAbove(QModelIndex()), QModelIndex());

    QStack<QModelIndex> parents;
    parents.push(QModelIndex());
    while (!parents.isEmpty()) {
        QModelIndex p = parents.pop();
        int rows = model.rowCount(p);
        for (int r = rows - 1; r > 0; --r) {
            QModelIndex idx = model.index(r, 0, p);
            QModelIndex expected = model.index(r - 1, 0, p);
            QCOMPARE(view.indexAbove(idx), expected);
        }
        // hide even rows
        for (int r = 0; r < rows; r+=2)
            view.setRowHidden(r, p, true);
        for (int r = rows - 1; r > 0; r-=2) {
            QModelIndex idx = model.index(r, 0, p);
            QModelIndex expected = model.index(r - 2, 0, p);
            QCOMPARE(view.indexAbove(idx), expected);
        }
//        for (int r = 0; r < rows; ++r)
//            parents.push(model.index(r, 0, p));
    }
}

void tst_QTreeView::indexBelow()
{
    QtTestModel model(2, 1);

    QTreeView view;
    view.setModel(&model);
    view.show();

    QModelIndex i = model.index(0, 0, view.rootIndex());
    QVERIFY(i.isValid());
    QCOMPARE(i.row(), 0);

    i = view.indexBelow(i);
    QVERIFY(i.isValid());
    QCOMPARE(i.row(), 1);
#if QT_VERSION >= 0x040100
    i = view.indexBelow(i);
    QVERIFY(!i.isValid());
#else
    // Qt 4.0.x returns the bottom index
    i = view.indexBelow(i);
    QVERIFY(i.isValid());
#endif
}

void tst_QTreeView::indexAt()
{
    QtTestModel model;
    model.rows = model.cols = 5;

    QTreeView view;
    QCOMPARE(view.indexAt(QPoint()), QModelIndex());
    view.setModel(&model);
    QVERIFY(view.indexAt(QPoint()) != QModelIndex());

    QSize itemSize = view.visualRect(model.index(0, 0)).size();
    for (int i = 0; i < model.rowCount(); ++i) {
        QPoint pos(itemSize.width() / 2, (i * itemSize.height()) + (itemSize.height() / 2));
        QModelIndex index = view.indexAt(pos);
        QCOMPARE(index, model.index(i, 0));
    }

    for (int j = 0; j < model.rowCount(); ++j)
        view.setIndexWidget(model.index(j, 0), new QPushButton);
    for (int k = 0; k < model.rowCount(); ++k) {
        QPoint pos(itemSize.width() / 2, (k * itemSize.height()) + (itemSize.height() / 2));
        QModelIndex index = view.indexAt(pos);
        QCOMPARE(index, model.index(k, 0));
    }
}

void tst_QTreeView::clicked()
{
    QtTestModel model(10, 2);

    QTreeView view;
    view.setModel(&model);

    view.show();
    QApplication::processEvents();

    QModelIndex firstIndex = model.index(0, 0, QModelIndex());
    QVERIFY(firstIndex.isValid());
    int itemHeight = view.visualRect(firstIndex).height();
    int itemOffset = view.visualRect(firstIndex).width() / 2;
    view.resize(200, itemHeight * (model.rows + 2));

    for (int i = 0; i < model.rows; ++i) {
        QPoint p(itemOffset, 1 + itemHeight * i);
        QModelIndex index = view.indexAt(p);
        if (!index.isValid())
            continue;
        QSignalSpy spy(&view, SIGNAL(clicked(const QModelIndex&)));
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
        QApplication::processEvents();
        QCOMPARE(spy.count(), 1);
    }
}

void tst_QTreeView::mouseDoubleClick()
{
    // Test double clicks outside the viewport.
    // (Should be a no-op and should not expand any item.)

    QStandardItemModel model(20, 2);
    for (int i = 0; i < model.rowCount(); i++) {
        QModelIndex index = model.index(i, 0, QModelIndex());
        model.insertRows(0, 20, index);
        model.insertColumns(0,2,index);
        for (int i1 = 0; i1 <  model.rowCount(index); i1++) {
            QModelIndex index2 = model.index(i1, 0, index);
        }
    }

    QTreeView view;
    view.setModel(&model);

    // make sure the viewport height is smaller than the contents height.
    view.resize(200,200);
    view.move(0,0);
    view.show();
    QModelIndex index = model.index(0, 0, QModelIndex());
    view.setCurrentIndex(index);

    view.setExpanded(model.index(0,0, QModelIndex()), true);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Make sure all items are collapsed
    for (int i = 0; i < model.rowCount(QModelIndex()); i++) {
        view.setExpanded(model.index(i,0, QModelIndex()), false);
    }

    int maximum = view.verticalScrollBar()->maximum();

    // Doubleclick in the bottom right corner, in the unused area between the vertical and horizontal scrollbar.
    int vsw = view.verticalScrollBar()->width();
    int hsh = view.horizontalScrollBar()->height();
    QTest::mouseDClick(&view, Qt::LeftButton, Qt::NoModifier, QPoint(view.width() - vsw + 1, view.height() - hsh + 1));
    // Should not have expanded, thus maximum() should have the same value.
    QCOMPARE(maximum, view.verticalScrollBar()->maximum());

}

void tst_QTreeView::rowsAboutToBeRemoved()
{
    QStandardItemModel model(3, 1);
    for (int i = 0; i < model.rowCount(); i++) {
        QModelIndex index = model.index(i, 0, QModelIndex());
        model.setData(index, QString("%1").arg(i));
        model.insertRows(0, 4, index);
        model.insertColumns(0,1,index);
        for (int i1 = 0; i1 <  model.rowCount(index); i1++) {
            QModelIndex index2 = model.index(i1, 0, index);
            model.setData(index2, QString("%1%2").arg(i).arg(i1));
        }
    }

    QTreeView view;
    view.setModel(&model);
    view.show();
    QModelIndex index = model.index(0,0, QModelIndex());
    view.setCurrentIndex(index);
    view.setExpanded(model.index(0,0, QModelIndex()), true);

    for (int i = 0; i < model.rowCount(QModelIndex()); i++) {
        view.setExpanded(model.index(i,0, QModelIndex()), true);
    }

    QSignalSpy spy1(&model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));

    model.removeRows(1,1);
    // Should not be 5 (or any other number for that sake :)
    QCOMPARE(spy1.count(), 1);

}

void tst_QTreeView::headerSections_unhideSection()
{
    QtTestModel model(10, 7);

    QTreeView view;

    view.setModel(&model);
    view.show();
    QApplication::processEvents();
    int size = view.header()->sectionSize(0);
    view.setColumnHidden(0, true);
    view.show();
    QApplication::processEvents();

    // should go back to old size
    view.setColumnHidden(0, false);
    QCOMPARE(size, view.header()->sectionSize(0));

}

void tst_QTreeView::columnAt()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.resize(500,500);
    view.setModel(&model);

    QCOMPARE(view.columnAt(0), 0);
    // really this is testing the header... so not much more should be needed if the header is working...
}

void tst_QTreeView::scrollTo()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.scrollTo(QModelIndex(), QTreeView::PositionAtTop);
    view.setModel(&model);

    // ### check the scrollbar values an make sure it actually scrolls to the item
    // ### check for bot item based and pixel based scrolling
    // ### create a data function for this test

    view.scrollTo(QModelIndex());
    view.scrollTo(model.index(0,0,QModelIndex()));
    view.scrollTo(model.index(0,0,QModelIndex()), QTreeView::PositionAtTop);
    view.scrollTo(model.index(0,0,QModelIndex()), QTreeView::PositionAtBottom);

    //

    view.show();
    view.resize(200, 20);
    QApplication::processEvents();
    view.verticalScrollBar()->setValue(0);

    view.scrollTo(model.index(0,0,QModelIndex()));
    QVERIFY(!view.visualRect(model.index(0,0,QModelIndex())).isEmpty()); // item is visible
    QCOMPARE(view.verticalScrollBar()->value(), 0);

    view.header()->resizeSection(0, 5); // now we only see the branches
    view.scrollTo(model.index(5, 0, QModelIndex()), QTreeView::PositionAtTop);
    QCOMPARE(view.verticalScrollBar()->value(), 5);

    // TODO force it to move to the left and then the right
}

void tst_QTreeView::rowsAboutToBeRemoved_move()
{
    QStandardItemModel model(3,1);
    QTreeView view;
    view.setModel(&model);
    QModelIndex indexThatWantsToLiveButWillDieDieITellYou;
    QModelIndex parent = model.index(2, 0 );
    view.expand(parent);
    for (int i = 0; i < 6; ++i) {
        model.insertRows(0, 1, parent);
        model.insertColumns(0, 1, parent);
        QModelIndex index = model.index(0, 0, parent);
        view.expand(index);
        if ( i == 3 )
            indexThatWantsToLiveButWillDieDieITellYou = index;
        model.setData(index, i);
        parent = index;
    }
    view.resize(600,800);
    view.show();
    view.doItemsLayout();
    static_cast<PublicView *>(&view)->executeDelayedItemsLayout();
    parent = indexThatWantsToLiveButWillDieDieITellYou.parent();
    QCOMPARE(view.isExpanded(indexThatWantsToLiveButWillDieDieITellYou), true);
    QCOMPARE(parent.isValid(), true);
    QCOMPARE(parent.parent().isValid(), true);
    view.expand(parent);
    QCOMPARE(view.isExpanded(parent), true);
    QCOMPARE(view.isExpanded(indexThatWantsToLiveButWillDieDieITellYou), true);
    model.removeRow(0, indexThatWantsToLiveButWillDieDieITellYou);
    QCOMPARE(view.isExpanded(parent), true);
    QCOMPARE(view.isExpanded(indexThatWantsToLiveButWillDieDieITellYou), true);
}

void tst_QTreeView::resizeColumnToContents()
{
    QStandardItemModel model(50,2);
    for (int r = 0; r < model.rowCount(); ++r) {
        for (int c = 0; c < model.columnCount(); ++c) {
            QModelIndex idx = model.index(r, c);
            model.setData(idx, QString::fromAscii("%1,%2").arg(r).arg(c) );
            model.insertColumns(0, 2, idx);
            model.insertRows(0, 6, idx);
            for (int i = 0; i < 6; ++i) {
                for (int j = 0; j < 2 ; ++j) {
                    model.setData(model.index(i, j, idx), QString::fromAscii("child%1%2").arg(i).arg(j));
                }
            }
        }
    }
    QTreeView view;
    view.setModel(&model);
    view.show();
    qApp->processEvents(); //must have this, or else it will not scroll
    view.scrollToBottom();
    view.resizeColumnToContents(0);
    int oldColumnSize = view.header()->sectionSize(0);
    view.setRootIndex(model.index(0, 0));
    view.resizeColumnToContents(0);        //Earlier, this gave an assert
    QVERIFY(view.header()->sectionSize(0) > oldColumnSize);
}

void tst_QTreeView::insertAfterSelect()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.setModel(&model);
    view.show();
    QModelIndex firstIndex = model.index(0, 0, QModelIndex());
    QVERIFY(firstIndex.isValid());
    int itemOffset = view.visualRect(firstIndex).width() / 2;
    QPoint p(itemOffset, 1);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    QVERIFY(view.selectionModel()->isSelected(firstIndex));
    model.insertNewRow();
    QVERIFY(view.selectionModel()->isSelected(firstIndex)); // Should still be selected
}

void tst_QTreeView::removeAfterSelect()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.setModel(&model);
    view.show();
    QModelIndex firstIndex = model.index(0, 0, QModelIndex());
    QVERIFY(firstIndex.isValid());
    int itemOffset = view.visualRect(firstIndex).width() / 2;
    QPoint p(itemOffset, 1);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    QVERIFY(view.selectionModel()->isSelected(firstIndex));
    model.removeLastRow();
    QVERIFY(view.selectionModel()->isSelected(firstIndex)); // Should still be selected
}

void tst_QTreeView::hiddenItems()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.setModel(&model);
    view.show();

    QModelIndex firstIndex = model.index(1, 0, QModelIndex());
    QVERIFY(firstIndex.isValid());
    for (int i=0; i < model.rowCount(firstIndex); i++)
        view.setRowHidden(i , firstIndex, true );

    int itemOffset = view.visualRect(firstIndex).width() / 2;
    int itemHeight = view.visualRect(firstIndex).height();
    QPoint p(itemOffset, itemHeight + 1);
    view.setExpanded(firstIndex, false);
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    QApplication::processEvents();
    QCOMPARE(view.isExpanded(firstIndex), false);

    p.setX( 5 );
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    QApplication::processEvents();
    QCOMPARE(view.isExpanded(firstIndex), false);
}

QTEST_MAIN(tst_QTreeView)
#include "tst_qtreeview.moc"
