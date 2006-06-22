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
    void noModel();
    void emptyModel();
    void removeRows();
    void removeCols();
    void expandAndCollapse();
    void keyboardNavigation();
    void headerSections();
    void moveCursor();
    void indexBelow();
    void clicked();
    void mouseDoubleClick();
    void rowsAboutToBeRemoved();
    void headerSections_unhideSection();
    void indentation();
    void rootIsDecorated();
    void uniformRowHeights();
    void itemsExpandable();
    void columnAt();
    void rowHidden();
    void scrollTo();
    void indexAt();
    void indexAbove();
    void rowsAboutToBeRemoved_move();
    void resizeColumnToContents();
    void insertAfterSelect();
    void removeAfterSelect();
};

class QtTestModel: public QAbstractItemModel
{
public:
    QtTestModel(QObject *parent = 0): QAbstractItemModel(parent),
       cols(0), rows(0), wrongIndex(false) {}
    int rowCount(const QModelIndex& parent = QModelIndex()) const { Q_UNUSED(parent); return rows; }
    int columnCount(const QModelIndex& parent = QModelIndex()) const { Q_UNUSED(parent); return cols; }
    bool isEditable(const QModelIndex &) const { return true; }

    mutable QMap<QModelIndex,QModelIndex> parentHash;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const
    {
        if (row < 0 || column < 0 || column >= cols || row >= rows) {
            return QModelIndex();
        }
        qint64 level = parent.isValid() ? parent.internalId() : -1;
        QModelIndex i = createIndex(row, column, ++level);
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
        if (role != Qt::DisplayRole)
            return QVariant();

        if (idx.row() < 0 || idx.column() < 0 || idx.column() >= cols || idx.row() >= rows) {
            wrongIndex = true;
            qWarning("Invalid modelIndex [%d,%d,%p]", idx.row(), idx.column(),
		     idx.internalPointer());
        }
        qint64 level = idx.internalId();
        return QString("[%1,%2,%3]").arg(idx.row()).arg(idx.column()).arg(level);
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

    int cols, rows;
    mutable bool wrongIndex;
};

// Testing get/set functions
void tst_QTreeView::getSetCheck()
{
    QTreeView obj1;
    // QHeaderView * QTreeView::header()
    // void QTreeView::setHeader(QHeaderView *)
    QHeaderView *var1 = new QHeaderView(Qt::Horizontal);
    obj1.setHeader(var1);
    QCOMPARE(var1, obj1.header());
#if QT_VERSION >= 0x040200
    // Itemviews in Qt < 4.2 have asserts for this. Qt >= 4.2 should handle this gracefully
    obj1.setHeader((QHeaderView *)0);
    QCOMPARE(var1, obj1.header());
#endif
    delete var1;

    // int QTreeView::indentation()
    // void QTreeView::setIndentation(int)
    obj1.setIndentation(0);
    QCOMPARE(0, obj1.indentation());
    obj1.setIndentation(INT_MIN);
    QCOMPARE(INT_MIN, obj1.indentation());
    obj1.setIndentation(INT_MAX);
    QCOMPARE(INT_MAX, obj1.indentation());

    // bool QTreeView::rootIsDecorated()
    // void QTreeView::setRootIsDecorated(bool)
    obj1.setRootIsDecorated(false);
    QCOMPARE(false, obj1.rootIsDecorated());
    obj1.setRootIsDecorated(true);
    QCOMPARE(true, obj1.rootIsDecorated());

    // bool QTreeView::uniformRowHeights()
    // void QTreeView::setUniformRowHeights(bool)
    obj1.setUniformRowHeights(false);
    QCOMPARE(false, obj1.uniformRowHeights());
    obj1.setUniformRowHeights(true);
    QCOMPARE(true, obj1.uniformRowHeights());

    // bool QTreeView::itemsExpandable()
    // void QTreeView::setItemsExpandable(bool)
    obj1.setItemsExpandable(false);
    QCOMPARE(false, obj1.itemsExpandable());
    obj1.setItemsExpandable(true);
    QCOMPARE(true, obj1.itemsExpandable());
}

tst_QTreeView::tst_QTreeView()
{
}

tst_QTreeView::~tst_QTreeView()
{
}

void tst_QTreeView::initTestCase()
{
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

void tst_QTreeView::noModel()
{
    QTreeView view;
    view.show();
    view.setRowHidden(0, QModelIndex(), true);
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
    QtTestModel model(0);
    model.rows = model.cols = 10;

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
    QtTestModel model(0);
    model.rows = model.cols = 10;

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
    QtTestModel model(0);
    model.rows = model.cols = 10;

    QTreeView view;
    view.setModel(&model);
    view.show();

    QModelIndex a = model.index(0, 0, QModelIndex());
    QModelIndex b = model.index(0, 0, a);

    // Test bad args
    view.expand(QModelIndex());
    view.collapse(QModelIndex());

    // expand an first level item
    view.expand(a);
    QVERIFY(view.isExpanded(a));

    // expand a seccond level itme
    view.expand(b);
    QVERIFY(view.isExpanded(a));
    QVERIFY(view.isExpanded(b));

    // collapse the first level item
    view.collapse(a);
    QVERIFY(!view.isExpanded(a));
    QVERIFY(view.isExpanded(b));

    // expand the first level item again
    view.expand(a);
    QVERIFY(view.isExpanded(a));
    QVERIFY(view.isExpanded(b));

    // collapse the seccond level item
    view.collapse(b);
    QVERIFY(view.isExpanded(a));
    QVERIFY(!view.isExpanded(b));

    // collapse the first level item
    view.collapse(a);
    QVERIFY(!view.isExpanded(a));
    QVERIFY(!view.isExpanded(b));

    // expand and remove row
    QPersistentModelIndex c = model.index(9, 0, b);
    view.expand(a);
    view.expand(b);
    model.removeLastRow(); // remove c
    QVERIFY(view.isExpanded(a));
    QVERIFY(view.isExpanded(b));
    QVERIFY(!view.isExpanded(c));

    // check signals
    qRegisterMetaType<QModelIndex>("QModelIndex");
    QSignalSpy spy_expanded(&view, SIGNAL(expanded(const QModelIndex&)));
    QSignalSpy spy_collapsed(&view, SIGNAL(collapsed(const QModelIndex&)));
    view.collapse(a);
    QCOMPARE(spy_expanded.count(), 0);
    QCOMPARE(spy_collapsed.count(), 1);
    spy_expanded.clear();
    spy_collapsed.clear();
    view.expand(a);
    QCOMPARE(spy_expanded.count(), 1);
    QCOMPARE(spy_collapsed.count(), 0);
}

static void populateModel(int rows, int columns, int level,
			  QStandardItemModel *model, QModelIndex root)
{
    if (rows <= 0)
	return;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QModelIndex index = model->index(i, j, root);
            model->setData(index,
			   QString("[%1,%2,%3]").arg(level).arg(i).arg(j));
        }
	populateModel(rows - 1, columns, level + 1, model,
		      model->index(i, 0, root));
    }
}

void tst_QTreeView::keyboardNavigation()
{
    int rows = 10;
    int columns = 10;

    QtTestModel model;
    model.rows = rows;
    model.cols = columns;

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
        case Qt::Key_Left:
	    QVERIFY(!view.isExpanded(index));
            break;
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
    QtTestModel model;
    model.rows = model.cols = 10;

    PublicView view;
    view.setModel(&model);
    view.setRowHidden(0, QModelIndex(), true);
    view.setColumnHidden(0, true);
    view.show();

    QModelIndex actual = view.moveCursor(PublicView::MoveDown, Qt::NoModifier);
    QModelIndex expected = model.index(1, 1, QModelIndex());

    QCOMPARE(actual, expected);
}

void tst_QTreeView::indexBelow()
{
    QtTestModel model;
    model.rows = 2;
    model.cols = 1;

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

void tst_QTreeView::clicked()
{
    QtTestModel model;
    model.rows = 10;
    model.cols = 2;

    qRegisterMetaType<QModelIndex>("QModelIndex");

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
    QtTestModel model;
    model.rows = 20;
    model.cols = 2;

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

    qRegisterMetaType<QModelIndex>("QModelIndex");
    QSignalSpy spy1(&model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));

    model.removeRows(1,1);
    // Should not be 5 (or any other number for that sake :)
    QCOMPARE(spy1.count(), 1);

}

void tst_QTreeView::headerSections_unhideSection()
{
    QtTestModel model;
    model.rows = model.cols = 10;

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

void tst_QTreeView::indentation()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.setModel(&model);
    QCOMPARE(view.indentation(), 20);
    view.setIndentation(10);
    QCOMPARE(view.indentation(), 10);
}

void tst_QTreeView::rootIsDecorated()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.setModel(&model);

    QCOMPARE(view.rootIsDecorated(), true);
    view.setRootIsDecorated(false);
    QCOMPARE(view.rootIsDecorated(), false);
    view.setRootIsDecorated(true);
    QCOMPARE(view.rootIsDecorated(), true);
}

void tst_QTreeView::uniformRowHeights()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.setModel(&model);

    QCOMPARE(view.uniformRowHeights(), false);
    view.setUniformRowHeights(true);
    QCOMPARE(view.uniformRowHeights(), true);
    view.setUniformRowHeights(false);
    QCOMPARE(view.uniformRowHeights(), false);
}

void tst_QTreeView::itemsExpandable()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.setModel(&model);

    QCOMPARE(view.itemsExpandable(), true);
    view.setItemsExpandable(false);
    QCOMPARE(view.itemsExpandable(), false);
    view.setItemsExpandable(true);
    QCOMPARE(view.itemsExpandable(), true);
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

void tst_QTreeView::rowHidden()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.resize(500,500);
    view.setModel(&model);

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

}

void tst_QTreeView::scrollTo()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    view.scrollTo(QModelIndex(), QTreeView::PositionAtTop);
    view.setModel(&model);

    view.scrollTo(QModelIndex());
    view.scrollTo(model.index(0,0,QModelIndex()));
    view.scrollTo(model.index(0,0,QModelIndex()), QTreeView::PositionAtTop);
    view.scrollTo(model.index(0,0,QModelIndex()), QTreeView::PositionAtBottom);
    // TODO force it to move to the left and then the right
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

void tst_QTreeView::indexAbove()
{
    QtTestModel model;
    model.rows = model.cols = 10;
    QTreeView view;
    QCOMPARE(view.indexAbove(QModelIndex()), QModelIndex());
    view.setModel(&model);
    QCOMPARE(view.indexAbove(QModelIndex()), QModelIndex());
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

QTEST_MAIN(tst_QTreeView)
#include "tst_qtreeview.moc"
