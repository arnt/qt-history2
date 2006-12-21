/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qabstractitemview.h>
#include <qstandarditemmodel.h>
#include <qapplication.h>
#include <qlistview.h>
#include <qtableview.h>
#include <qtreeview.h>
#include <qheaderview.h>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qabstractitemview.h gui/itemviews/qabstractitemview.cpp

class TestView : public QAbstractItemView
{
    Q_OBJECT
public:
    inline void tst_dataChanged(const QModelIndex &tl, const QModelIndex &br)
        { dataChanged(tl, br); }
    inline void tst_setHorizontalStepsPerItem(int steps)
        { setHorizontalStepsPerItem(steps); }
    inline int tst_horizontalStepsPerItem() const
        { return horizontalStepsPerItem(); }
    inline void tst_setVerticalStepsPerItem(int steps)
        { setVerticalStepsPerItem(steps); }
    inline int tst_verticalStepsPerItem() const
        { return verticalStepsPerItem(); }

    inline void tst_rowsInserted(const QModelIndex &parent, int start, int end)
        { rowsInserted(parent, start, end); }
    inline void tst_rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
        { rowsAboutToBeRemoved(parent, start, end); }
    inline void tst_selectionChanged(const QItemSelection &selected,
                                     const QItemSelection &deselected)
        { selectionChanged(selected, deselected); }
    inline void tst_currentChanged(const QModelIndex &current, const QModelIndex &previous)
        { currentChanged(current, previous); }
    inline void tst_updateEditorData()
        { updateEditorData(); }
    inline void tst_updateEditorGeometries()
        { updateEditorGeometries(); }
    inline void tst_updateGeometries()
        { updateGeometries(); }
    inline void tst_verticalScrollbarAction(int action)
        { verticalScrollbarAction(action); }
    inline void tst_horizontalScrollbarAction(int action)
        { horizontalScrollbarAction(action); }
    inline void tst_verticalScrollbarValueChanged(int value)
        { verticalScrollbarValueChanged(value); }
    inline void tst_horizontalScrollbarValueChanged(int value)
        { horizontalScrollbarValueChanged(value); }
    inline void tst_closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
        { closeEditor(editor, hint); }
    inline void tst_commitData(QWidget *editor)
        { commitData(editor); }
    inline void tst_editorDestroyed(QObject *editor)
        { editorDestroyed(editor); }
    enum tst_CursorAction {
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
    inline QModelIndex tst_moveCursor(tst_CursorAction cursorAction,
                                      Qt::KeyboardModifiers modifiers)
        { return moveCursor(QAbstractItemView::CursorAction(cursorAction), modifiers); }
    inline int tst_horizontalOffset() const
        { return horizontalOffset(); }
    inline int tst_verticalOffset() const
        { return verticalOffset(); }
    inline bool tst_isIndexHidden(const QModelIndex &index) const
        { return isIndexHidden(index); }
    inline void tst_setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
        { setSelection(rect, command); }
    inline QRegion tst_visualRegionForSelection(const QItemSelection &selection) const
        { return visualRegionForSelection(selection); }
    inline QModelIndexList tst_selectedIndexes() const
        { return selectedIndexes(); }
    inline bool tst_edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
        { return edit(index, trigger, event); }
    inline QItemSelectionModel::SelectionFlags tst_selectionCommand(const QModelIndex &index,
                                                                    const QEvent *event = 0) const
        { return selectionCommand(index, event); }
    inline void tst_startDrag(Qt::DropActions supportedActions)
        { startDrag(supportedActions); }
    inline QStyleOptionViewItem tst_viewOptions() const
        { return viewOptions(); }
    enum tst_State {
        NoState = QAbstractItemView::NoState,
        DraggingState = QAbstractItemView::DraggingState,
        DragSelectingState = QAbstractItemView::DragSelectingState,
        EditingState = QAbstractItemView::EditingState,
        ExpandingState = QAbstractItemView::ExpandingState,
        CollapsingState = QAbstractItemView::CollapsingState
    };
    inline tst_State tst_state() const
        { return (tst_State)state(); }
    inline void tst_setState(tst_State state)
        { setState(QAbstractItemView::State(state)); }
    inline void tst_startAutoScroll()
        { startAutoScroll(); }
    inline void tst_stopAutoScroll()
        { stopAutoScroll(); }
    inline void tst_doAutoScroll()
        { doAutoScroll(); }
};

class tst_QAbstractItemView : public QObject
{
    Q_OBJECT

public:

    tst_QAbstractItemView();
    virtual ~tst_QAbstractItemView();
    void basic_tests(TestView *view);

private slots:
    void getSetCheck();
    void emptyModels_data();
    void emptyModels();
    void setModel_data();
    void setModel();
    void noModel();
    void dragSelect();
    void rowDelegate();
    void columnDelegate();
    void selectAll();
    void ctrlA();
    void dragAndDrop();
};

class MyAbstractItemDelegate : public QAbstractItemDelegate
{
public:
    MyAbstractItemDelegate() : QAbstractItemDelegate() {};
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const {}
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const { return QSize(); }
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
                          const QModelIndex &) const { return new QWidget(parent); }
};

// Testing get/set functions
void tst_QAbstractItemView::getSetCheck()
{
    QListView view;
    TestView *obj1 = reinterpret_cast<TestView*>(&view);
    // QAbstractItemDelegate * QAbstractItemView::itemDelegate()
    // void QAbstractItemView::setItemDelegate(QAbstractItemDelegate *)
    MyAbstractItemDelegate *var1 = new MyAbstractItemDelegate;
    obj1->setItemDelegate(var1);
    QCOMPARE((QAbstractItemDelegate*)var1, obj1->itemDelegate());
#if QT_VERSION >= 0x040200
    // Itemviews in Qt < 4.2 have asserts for this. Qt >= 4.2 should handle this gracefully
    obj1->setItemDelegate((QAbstractItemDelegate *)0);
    QCOMPARE((QAbstractItemDelegate *)0, obj1->itemDelegate());
#endif
    delete var1;

    // EditTriggers QAbstractItemView::editTriggers()
    // void QAbstractItemView::setEditTriggers(EditTriggers)
    obj1->setEditTriggers(QAbstractItemView::EditTriggers(QAbstractItemView::NoEditTriggers));
    QCOMPARE(QAbstractItemView::EditTriggers(QAbstractItemView::NoEditTriggers), obj1->editTriggers());
    obj1->setEditTriggers(QAbstractItemView::EditTriggers(QAbstractItemView::CurrentChanged));
    QCOMPARE(QAbstractItemView::EditTriggers(QAbstractItemView::CurrentChanged), obj1->editTriggers());
    obj1->setEditTriggers(QAbstractItemView::EditTriggers(QAbstractItemView::DoubleClicked));
    QCOMPARE(QAbstractItemView::EditTriggers(QAbstractItemView::DoubleClicked), obj1->editTriggers());
    obj1->setEditTriggers(QAbstractItemView::EditTriggers(QAbstractItemView::SelectedClicked));
    QCOMPARE(QAbstractItemView::EditTriggers(QAbstractItemView::SelectedClicked), obj1->editTriggers());
    obj1->setEditTriggers(QAbstractItemView::EditTriggers(QAbstractItemView::EditKeyPressed));
    QCOMPARE(QAbstractItemView::EditTriggers(QAbstractItemView::EditKeyPressed), obj1->editTriggers());
    obj1->setEditTriggers(QAbstractItemView::EditTriggers(QAbstractItemView::AnyKeyPressed));
    QCOMPARE(QAbstractItemView::EditTriggers(QAbstractItemView::AnyKeyPressed), obj1->editTriggers());
    obj1->setEditTriggers(QAbstractItemView::EditTriggers(QAbstractItemView::AllEditTriggers));
    QCOMPARE(QAbstractItemView::EditTriggers(QAbstractItemView::AllEditTriggers), obj1->editTriggers());

    // bool QAbstractItemView::tabKeyNavigation()
    // void QAbstractItemView::setTabKeyNavigation(bool)
    obj1->setTabKeyNavigation(false);
    QCOMPARE(false, obj1->tabKeyNavigation());
    obj1->setTabKeyNavigation(true);
    QCOMPARE(true, obj1->tabKeyNavigation());

    // bool QAbstractItemView::dragEnabled()
    // void QAbstractItemView::setDragEnabled(bool)
    obj1->setDragEnabled(false);
    QCOMPARE(false, obj1->dragEnabled());
    obj1->setDragEnabled(true);
    QCOMPARE(true, obj1->dragEnabled());

    // bool QAbstractItemView::alternatingRowColors()
    // void QAbstractItemView::setAlternatingRowColors(bool)
    obj1->setAlternatingRowColors(false);
    QCOMPARE(false, obj1->alternatingRowColors());
    obj1->setAlternatingRowColors(true);
    QCOMPARE(true, obj1->alternatingRowColors());

#if QT_VERSION < 0x040200
    // int QAbstractItemView::horizontalStepsPerItem()
    // void QAbstractItemView::setHorizontalStepsPerItem(int)
    obj1->tst_setHorizontalStepsPerItem(0);
    QCOMPARE(0, obj1->tst_horizontalStepsPerItem());
    obj1->tst_setHorizontalStepsPerItem(INT_MIN);
    QCOMPARE(INT_MIN, obj1->tst_horizontalStepsPerItem());
    obj1->tst_setHorizontalStepsPerItem(INT_MAX);
    QCOMPARE(INT_MAX, obj1->tst_horizontalStepsPerItem());

    // int QAbstractItemView::verticalStepsPerItem()
    // void QAbstractItemView::setVerticalStepsPerItem(int)
    obj1->tst_setVerticalStepsPerItem(0);
    QCOMPARE(0, obj1->tst_verticalStepsPerItem());
    obj1->tst_setVerticalStepsPerItem(INT_MIN);
    QCOMPARE(INT_MIN, obj1->tst_verticalStepsPerItem());
    obj1->tst_setVerticalStepsPerItem(INT_MAX);
    QCOMPARE(INT_MAX, obj1->tst_verticalStepsPerItem());
#endif

    // State QAbstractItemView::state()
    // void QAbstractItemView::setState(State)
    obj1->tst_setState(TestView::tst_State(TestView::NoState));
    QCOMPARE(TestView::tst_State(TestView::NoState), obj1->tst_state());
    obj1->tst_setState(TestView::tst_State(TestView::DraggingState));
    QCOMPARE(TestView::tst_State(TestView::DraggingState), obj1->tst_state());
    obj1->tst_setState(TestView::tst_State(TestView::DragSelectingState));
    QCOMPARE(TestView::tst_State(TestView::DragSelectingState), obj1->tst_state());
    obj1->tst_setState(TestView::tst_State(TestView::EditingState));
    QCOMPARE(TestView::tst_State(TestView::EditingState), obj1->tst_state());
    obj1->tst_setState(TestView::tst_State(TestView::ExpandingState));
    QCOMPARE(TestView::tst_State(TestView::ExpandingState), obj1->tst_state());
    obj1->tst_setState(TestView::tst_State(TestView::CollapsingState));
    QCOMPARE(TestView::tst_State(TestView::CollapsingState), obj1->tst_state());

#if QT_VERSION >= 0x040200
    // QWidget QAbstractScrollArea::viewport()
    // void setViewport(QWidget*)
    QWidget *vp = new QWidget;
    obj1->setViewport(vp);
    QCOMPARE(vp, obj1->viewport());
#endif
}

tst_QAbstractItemView::tst_QAbstractItemView()
{
}

tst_QAbstractItemView::~tst_QAbstractItemView()
{
}

void tst_QAbstractItemView::emptyModels_data()
{
    QTest::addColumn<QString>("viewType");

    QTest::newRow("QListView") << "QListView";
    QTest::newRow("QTableView") << "QTableView";
    QTest::newRow("QTreeView") << "QTreeView";
    QTest::newRow("QHeaderView") << "QHeaderView";
}

void tst_QAbstractItemView::emptyModels()
{
    QFETCH(QString, viewType);

    TestView *view = 0;
    if (viewType == "QListView")
        view = reinterpret_cast<TestView*>(new QListView());
    else if (viewType == "QTableView")
        view = reinterpret_cast<TestView*>(new QTableView());
    else if (viewType == "QTreeView")
        view = reinterpret_cast<TestView*>(new QTreeView());
    else if (viewType == "QHeaderView")
        view = reinterpret_cast<TestView*>(new QHeaderView(Qt::Vertical));
    else
        QVERIFY(0);
    view->show();

    QVERIFY(!view->model());
    QVERIFY(!view->selectionModel());
    //QVERIFY(view->itemDelegate() != 0);

    basic_tests(view);
    delete view;
}

void tst_QAbstractItemView::setModel_data()
{
    QTest::addColumn<QString>("viewType");

    QTest::newRow("QListView") << "QListView";
    QTest::newRow("QTableView") << "QTableView";
    QTest::newRow("QTreeView") << "QTreeView";
    QTest::newRow("QHeaderView") << "QHeaderView";
}

void tst_QAbstractItemView::setModel()
{
    QFETCH(QString, viewType);
    TestView *view = 0;
    if (viewType == "QListView")
        view = reinterpret_cast<TestView*>(new QListView());
    else if (viewType == "QTableView")
        view = reinterpret_cast<TestView*>(new QTableView());
    else if (viewType == "QTreeView")
        view = reinterpret_cast<TestView*>(new QTreeView());
    else if (viewType == "QHeaderView")
        view = reinterpret_cast<TestView*>(new QHeaderView(Qt::Vertical));
    else
        QVERIFY(0);
    view->show();

    QStandardItemModel model(20,20);
    view->setModel(0);
    view->setModel(&model);
    basic_tests(view);
    delete view;
}

void tst_QAbstractItemView::basic_tests(TestView *view)
{
    // setSelectionModel
    // Will assert as it should
    //view->setSelectionModel(0);
    // setItemDelegate
    //view->setItemDelegate(0);
    // Will asswert as it should

    // setSelectionMode
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::SingleSelection);
    view->setSelectionMode(QAbstractItemView::ContiguousSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::ContiguousSelection);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::ExtendedSelection);
    view->setSelectionMode(QAbstractItemView::MultiSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::MultiSelection);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::NoSelection);

    // setSelectionBehavior
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    QCOMPARE(view->selectionBehavior(), QAbstractItemView::SelectItems);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    QCOMPARE(view->selectionBehavior(), QAbstractItemView::SelectRows);
    view->setSelectionBehavior(QAbstractItemView::SelectColumns);
    QCOMPARE(view->selectionBehavior(), QAbstractItemView::SelectColumns);

    // setEditTriggers
    view->setEditTriggers(QAbstractItemView::EditKeyPressed);
    QCOMPARE(view->editTriggers(), QAbstractItemView::EditKeyPressed);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QCOMPARE(view->editTriggers(), QAbstractItemView::NoEditTriggers);
    view->setEditTriggers(QAbstractItemView::CurrentChanged);
    QCOMPARE(view->editTriggers(), QAbstractItemView::CurrentChanged);
    view->setEditTriggers(QAbstractItemView::DoubleClicked);
    QCOMPARE(view->editTriggers(), QAbstractItemView::DoubleClicked);
    view->setEditTriggers(QAbstractItemView::SelectedClicked);
    QCOMPARE(view->editTriggers(), QAbstractItemView::SelectedClicked);
    view->setEditTriggers(QAbstractItemView::AnyKeyPressed);
    QCOMPARE(view->editTriggers(), QAbstractItemView::AnyKeyPressed);
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    QCOMPARE(view->editTriggers(), QAbstractItemView::AllEditTriggers);

    // setAutoScroll
    view->setAutoScroll(false);
    QCOMPARE(view->hasAutoScroll(), false);
    view->setAutoScroll(true);
    QCOMPARE(view->hasAutoScroll(), true);

    // setTabKeyNavigation
    view->setTabKeyNavigation(false);
    QCOMPARE(view->tabKeyNavigation(), false);
    view->setTabKeyNavigation(true);
    QCOMPARE(view->tabKeyNavigation(), true);

    // setDropIndicatorShown
    view->setDropIndicatorShown(false);
    QCOMPARE(view->showDropIndicator(), false);
    view->setDropIndicatorShown(true);
    QCOMPARE(view->showDropIndicator(), true);

    // setDragEnabled
    view->setDragEnabled(false);
    QCOMPARE(view->dragEnabled(), false);
    view->setDragEnabled(true);
    QCOMPARE(view->dragEnabled(), true);

    // setAlternatingRowColors
    view->setAlternatingRowColors(false);
    QCOMPARE(view->alternatingRowColors(), false);
    view->setAlternatingRowColors(true);
    QCOMPARE(view->alternatingRowColors(), true);

    // setIconSize
    view->setIconSize(QSize(16, 16));
    QCOMPARE(view->iconSize(), QSize(16, 16));
    view->setIconSize(QSize(32, 32));
    QCOMPARE(view->iconSize(), QSize(32, 32));
    // Should this happen?
    view->setIconSize(QSize(-1, -1));
    QCOMPARE(view->iconSize(), QSize(-1, -1));

    QCOMPARE(view->currentIndex(), QModelIndex());
    QCOMPARE(view->rootIndex(), QModelIndex());

    view->keyboardSearch("");
    view->keyboardSearch("foo");
    view->keyboardSearch("1");

    QCOMPARE(view->visualRect(QModelIndex()), QRect());

    view->scrollTo(QModelIndex());

    QCOMPARE(view->sizeHintForIndex(QModelIndex()), QSize());
    QCOMPARE(view->indexAt(QPoint(-1, -1)), QModelIndex());

    if (!view->model()){
        QCOMPARE(view->indexAt(QPoint(10, 10)), QModelIndex());
        QCOMPARE(view->sizeHintForRow(0), -1);
        QCOMPARE(view->sizeHintForColumn(0), -1);
    }else if (view->itemDelegate()){
        view->sizeHintForRow(0);
        view->sizeHintForColumn(0);
    }
    view->openPersistentEditor(QModelIndex());
    view->closePersistentEditor(QModelIndex());

    view->reset();
    view->setRootIndex(QModelIndex());
    view->doItemsLayout();
    view->selectAll();
    view->edit(QModelIndex());
    view->clearSelection();
    view->setCurrentIndex(QModelIndex());

    // protected methods
    view->tst_dataChanged(QModelIndex(), QModelIndex());
    view->tst_rowsInserted(QModelIndex(), -1, -1);
    view->tst_rowsAboutToBeRemoved(QModelIndex(), -1, -1);
    view->tst_selectionChanged(QItemSelection(), QItemSelection());
    if (view->model()){
        view->tst_currentChanged(QModelIndex(), QModelIndex());
        view->tst_currentChanged(QModelIndex(), view->model()->index(0,0));
    }
    view->tst_updateEditorData();
    view->tst_updateEditorGeometries();
    view->tst_updateGeometries();
    view->tst_verticalScrollbarAction(QAbstractSlider::SliderSingleStepAdd);
    view->tst_horizontalScrollbarAction(QAbstractSlider::SliderSingleStepAdd);
    view->tst_verticalScrollbarValueChanged(10);
    view->tst_horizontalScrollbarValueChanged(10);
    view->tst_closeEditor(0, QAbstractItemDelegate::NoHint);
    view->tst_commitData(0);
    view->tst_editorDestroyed(0);

    view->tst_setHorizontalStepsPerItem(2);
    view->tst_horizontalStepsPerItem();
    view->tst_setVerticalStepsPerItem(2);
    view->tst_verticalStepsPerItem();

    // Will assert as it should
    // view->setIndexWidget(QModelIndex(), 0);

    view->tst_moveCursor(TestView::MoveUp, Qt::NoModifier);
    view->tst_horizontalOffset();
    view->tst_verticalOffset();

//    view->tst_isIndexHidden(QModelIndex()); // will (correctly) assert
    if(view->model())
        view->tst_isIndexHidden(view->model()->index(0,0));

    view->tst_setSelection(QRect(0, 0, 10, 10), QItemSelectionModel::ClearAndSelect);
    view->tst_setSelection(QRect(-1, -1, -1, -1), QItemSelectionModel::ClearAndSelect);
    view->tst_visualRegionForSelection(QItemSelection());
    view->tst_selectedIndexes();

    view->tst_edit(QModelIndex(), QAbstractItemView::NoEditTriggers, 0);

    view->tst_selectionCommand(QModelIndex(), 0);

    if (!view->model())
        view->tst_startDrag(Qt::CopyAction);

    view->tst_viewOptions();

    view->tst_setState(TestView::NoState);
    QVERIFY(view->tst_state()==TestView::NoState);
    view->tst_setState(TestView::DraggingState);
    QVERIFY(view->tst_state()==TestView::DraggingState);
    view->tst_setState(TestView::DragSelectingState);
    QVERIFY(view->tst_state()==TestView::DragSelectingState);
    view->tst_setState(TestView::EditingState);
    QVERIFY(view->tst_state()==TestView::EditingState);
    view->tst_setState(TestView::ExpandingState);
    QVERIFY(view->tst_state()==TestView::ExpandingState);
    view->tst_setState(TestView::CollapsingState);
    QVERIFY(view->tst_state()==TestView::CollapsingState);

    view->tst_startAutoScroll();
    view->tst_stopAutoScroll();
    view->tst_doAutoScroll();

    // testing mouseFoo and key functions
//     QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, QPoint(0,0));
//     mouseMove(view, Qt::LeftButton, Qt::NoModifier, QPoint(10,10));
//     QTest::mouseRelease(view, Qt::LeftButton, Qt::NoModifier, QPoint(10,10));
//     QTest::mouseClick(view, Qt::LeftButton, Qt::NoModifier, QPoint(10,10));
//     mouseDClick(view, Qt::LeftButton, Qt::NoModifier, QPoint(10,10));
//     QTest::keyClick(view, Qt::Key_A);
}

void tst_QAbstractItemView::noModel()
{
    // From task #85415

    QStandardItemModel model(20,20);
    QTreeView view;

    view.setModel(&model);
    // Make the viewport smaller than the contents, so that we can scroll
    view.resize(100,100);
    view.show();

    // make sure that the scrollbars are not at value 0
    view.scrollTo(view.model()->index(10,10));
    QApplication::processEvents();

    view.setModel(0);
    // Due to the model is removed, this will generate a valueChanged signal on both scrollbars. (value to 0)
    QApplication::processEvents();
    QCOMPARE(view.model(), (QAbstractItemModel*)0);
}

void tst_QAbstractItemView::dragSelect()
{
    // From task #86108

    QStandardItemModel model(64,64);

    QTableView view;
    view.setModel(&model);
    view.setVisible(true);

    const int delay = 2;
    for (int i = 0; i < 2; ++i) {
        bool tracking = (i == 1);
        view.setMouseTracking(false);
        QTest::mouseMove(&view, QPoint(0, 0), delay);
        view.setMouseTracking(tracking);
        QTest::mouseMove(&view, QPoint(50, 50), delay);
        QVERIFY(view.selectionModel()->selectedIndexes().isEmpty());
    }
}

void tst_QAbstractItemView::rowDelegate()
{
    QStandardItemModel model(4,4);
    MyAbstractItemDelegate delegate;

    QTableView view;
    view.setModel(&model);
    view.setItemDelegateForRow(3, &delegate);
    view.show();

    QModelIndex index = model.index(3, 0);
    view.openPersistentEditor(index);
    QWidget *w = view.indexWidget(index);
    QVERIFY(w);
    QCOMPARE(w->metaObject()->className(), "QWidget");
}

void tst_QAbstractItemView::columnDelegate()
{
    QStandardItemModel model(4,4);
    MyAbstractItemDelegate delegate;

    QTableView view;
    view.setModel(&model);
    view.setItemDelegateForColumn(3, &delegate);
    view.show();

    QModelIndex index = model.index(0, 3);
    view.openPersistentEditor(index);
    QWidget *w = view.indexWidget(index);
    QVERIFY(w);
    QCOMPARE(w->metaObject()->className(), "QWidget");
}

void tst_QAbstractItemView::selectAll()
{
    QStandardItemModel model(4,4);
    QTableView view;
    view.setModel(&model);

    TestView *tst_view = (TestView*)&view;

    QCOMPARE(tst_view->tst_selectedIndexes().count(), 0);
    view.selectAll();
    QCOMPARE(tst_view->tst_selectedIndexes().count(), 4*4);
}

void tst_QAbstractItemView::ctrlA()
{
    QStandardItemModel model(4,4);
    QTableView view;
    view.setModel(&model);

    TestView *tst_view = (TestView*)&view;

    QCOMPARE(tst_view->tst_selectedIndexes().count(), 0);
    QTest::keyClick(&view, Qt::Key_A, Qt::ControlModifier);
    QCOMPARE(tst_view->tst_selectedIndexes().count(), 4*4);
}

#if defined(Q_WS_X11)
extern void qt_x11_wait_for_window_manager(QWidget *w);
#endif

static void sendMouseMove(QWidget *widget, QPoint pos = QPoint())
{
    if (pos.isNull())
        pos = widget->rect().center();
    QMouseEvent event(QEvent::MouseMove, pos, widget->mapToGlobal(pos), Qt::NoButton, 0, 0);
    QCursor::setPos(widget->mapToGlobal(pos));
    qApp->processEvents();
#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(widget);
#endif
    QApplication::sendEvent(widget, &event);
}

static void sendMousePress(
    QWidget *widget, QPoint pos = QPoint(), Qt::MouseButton button = Qt::LeftButton)
{
    if (pos.isNull())
         pos = widget->rect().center();
    QMouseEvent event(QEvent::MouseButtonPress, pos, widget->mapToGlobal(pos), button, 0, 0);
    QApplication::sendEvent(widget, &event);
}

static void sendMouseRelease(
    QWidget *widget, QPoint pos = QPoint(), Qt::MouseButton button = Qt::LeftButton)
{
    if (pos.isNull())
         pos = widget->rect().center();
    QMouseEvent event(QEvent::MouseButtonRelease, pos, widget->mapToGlobal(pos), button, 0, 0);
    QApplication::sendEvent(widget, &event);
}

class DnDTestModel : public QStandardItemModel
{
    Q_OBJECT
    bool dropMimeData(const QMimeData *, Qt::DropAction action, int, int, const QModelIndex &)
    {
        dropAction_result = action;
        return true;
    }
    Qt::DropActions supportedDropActions() const { return Qt::CopyAction | Qt::MoveAction; }

    Qt::DropAction dropAction_result;
public:
    DnDTestModel() : QStandardItemModel(20, 20) {}
    Qt::DropAction dropAction() const { return dropAction_result; }
};

class DnDTestView : public QListView
{
    Q_OBJECT

    Qt::DropAction dropAction;

    void timerEvent(QTimerEvent *event)
    {
        killTimer(event->timerId());
        sendMouseMove(this);
        sendMouseRelease(this);
    }

    void dragEnterEvent(QDragEnterEvent *event)
    {
        QAbstractItemView::dragEnterEvent(event);
        startTimer(0);
    }

    void dropEvent(QDropEvent *event)
    {
        event->setDropAction(dropAction);
        QListView::dropEvent(event);
    }

public:
    DnDTestView(Qt::DropAction dropAction, QAbstractItemModel *model)
        : dropAction(dropAction)
    {
        setModel(model);
        setDragDropMode(QAbstractItemView::DragDrop);
        setAcceptDrops(true);
        setDragEnabled(true);
    }
};

class DnDTestWidget : public QWidget
{
    Q_OBJECT

    Qt::DropAction dropAction_request;
    Qt::DropAction dropAction_result;
    QWidget *dropTarget;

    void mousePressEvent(QMouseEvent *)
    {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setData("application/x-qabstractitemmodeldatalist", QByteArray("foobar"));
        drag->setMimeData(mimeData);
        dropAction_result = drag->start(dropAction_request);
    }

public:
    Qt::DropAction dropAction() const { return dropAction_result; }

    void dragAndDrop(QWidget *dropTarget, Qt::DropAction dropAction)
    {
        this->dropTarget = dropTarget;
        dropAction_request = dropAction;
        sendMousePress(this);
    }
};

void tst_QAbstractItemView::dragAndDrop()
{
    // From Task 137729

    int successes = 0;
    for (int i = 0; i < 10; ++i) {
        Qt::DropAction dropAction = Qt::MoveAction;

        DnDTestModel model;
        DnDTestView view(dropAction, &model);
        DnDTestWidget widget;

        const int size = 200;
        widget.setFixedSize(size, size);
        view.setFixedSize(size, size);

        widget.move(0, 0);
        view.move(int(size * 1.5), int(size * 1.5));

        widget.show();
        view.show();
#if defined(Q_WS_X11)
        qt_x11_wait_for_window_manager(&widget);
        qt_x11_wait_for_window_manager(&view);
#endif

        widget.dragAndDrop(&view, dropAction);
        if (model.dropAction() == dropAction
            && widget.dropAction() == dropAction)
            ++successes;
    }

    QVERIFY(successes > 0); // allow for window manager "effects"
}

QTEST_MAIN(tst_QAbstractItemView)
#include "tst_qabstractitemview.moc"
