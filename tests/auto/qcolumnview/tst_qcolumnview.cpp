/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qstandarditemmodel.h>
#include <qcolumnview.h>
#include "../../../src/gui/itemviews/qcolumnviewgrip_p.h"
#include <qdirmodel.h>
#include <qstringlistmodel.h>
#include <qdebug.h>
#include <qitemdelegate.h>
#include <qscrollbar.h>
#include <private/qcolumnview_p.h>

//TESTED_CLASS=QColumnView
//TESTED_FILES=gui/itemviews/qcolumnview.h gui/itemviews/qcolumnview.cpp

// Will try to wait for the condition while allowing event processing
// for a maximum of 5 seconds.
#define TRY_VERIFY(expr) \
    do { \
        const int step = 50; \
        for (int i = 0; i < 5000 && !(expr); i+=step) { \
            QTest::qWait(step); \
        } \
        QVERIFY(expr); \
    } while(0)

#define ANIMATION_DELAY 300

class tst_QColumnView : public QObject {
  Q_OBJECT

public:
    tst_QColumnView();
    virtual ~tst_QColumnView();

public Q_SLOTS:
    void init();
    void cleanup();

private slots:
    void rootIndex();
    void grips();
    void isIndexHidden();
    void indexAt();
    void scrollContentsBy_data();
    void scrollContentsBy();
    void scrollTo();
    void moveCursor_data();
    void moveCursor();
    void selectAll();
    void clicked();
    void selectedColumns();
    void setSelection();
    void setSelectionModel();
    void visualRegionForSelection();

    // grip
    void moveGrip_basic();
    void moveGrip_data();
    void moveGrip();
    void doubleClick();
    void gripMoved();

    void preview();
    void swapPreview();
    void sizes();
    void rowDelegate();
    void resize();
    void changeSameColumn();

protected slots:
    void setPreviewWidget();
};

class ColumnViewPrivate : public QColumnViewPrivate
{
public:
    ColumnViewPrivate() : QColumnViewPrivate() {}
};

class ColumnView : public QColumnView {

public:
    ColumnView(QWidget *parent = 0) : QColumnView(*new ColumnViewPrivate, parent){}

    QList<QPointer<QAbstractItemView> > createdColumns;
    void ScrollContentsBy(int x, int y) {scrollContentsBy(x,y); }
    int HorizontalOffset() const { return horizontalOffset(); }

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

    inline QModelIndex MoveCursor(PublicCursorAction ca, Qt::KeyboardModifiers kbm)
        { return QColumnView::moveCursor((CursorAction)ca, kbm); }
    bool IsIndexHidden(const QModelIndex&index) const
        { return isIndexHidden(index); }

    void setSelection(const QRect & rect, QItemSelectionModel::SelectionFlags command )
    {
        QColumnView::setSelection(rect, command);
    }

    QRegion visualRegionForSelection(QItemSelection selection){
        return QColumnView::visualRegionForSelection(selection);
    }
protected:
    QAbstractItemView *createColumn(const QModelIndex &index) {
        QAbstractItemView *view = QColumnView::createColumn(index);
        QPointer<QAbstractItemView> savedView = view;
        createdColumns.append(savedView);
        return view;
    }

};

tst_QColumnView::tst_QColumnView()
{
}

tst_QColumnView::~tst_QColumnView()
{
}

void tst_QColumnView::init()
{
    qApp->setLayoutDirection(Qt::LeftToRight);
}

void tst_QColumnView::cleanup()
{
}

void tst_QColumnView::rootIndex()
{
    ColumnView view;
    // no model
    view.setRootIndex(QModelIndex());

    QDirModel model;
    view.setModel(&model);

    QModelIndex drive = model.index(0, 0);
    QVERIFY(view.visualRect(drive).isValid());
    view.setRootIndex(QModelIndex());
    QCOMPARE(view.HorizontalOffset(), 0);
    QCOMPARE(view.rootIndex(), QModelIndex());
    QVERIFY(view.visualRect(drive).isValid());

    QModelIndex home = model.index(QDir::homePath());
    QModelIndex homeFile = model.index(0, 0, home);
    int i = 0;
    while (i < model.rowCount(home) - 1 && !model.hasChildren(homeFile))
        homeFile = model.index(++i, 0, home);
    view.setRootIndex(home);
    QCOMPARE(view.HorizontalOffset(), 0);
    QCOMPARE(view.rootIndex(), home);
    QVERIFY(!view.visualRect(drive).isValid());
    QVERIFY(!view.visualRect(home).isValid());
    if (homeFile.isValid())
        QVERIFY(view.visualRect(homeFile).isValid());

    // set root when there already is one
    view.setRootIndex(home);
    view.setCurrentIndex(homeFile);
    view.scrollTo(model.index(0,0, homeFile));
    QCOMPARE(view.HorizontalOffset(), 0);
    QCOMPARE(view.rootIndex(), home);
    QVERIFY(!view.visualRect(drive).isValid());
    QVERIFY(!view.visualRect(home).isValid());
     if (homeFile.isValid())
        QVERIFY(view.visualRect(homeFile).isValid());

    //
    homeFile = model.index(QDir::currentPath());
    home = homeFile.parent();
    view.setRootIndex(home);
    view.setCurrentIndex(homeFile);
    view.show();
    i = 0;
    QModelIndex two = model.index(0, 0, homeFile);
    while (i < model.rowCount(homeFile) - 1 && !model.hasChildren(two))
        two = model.index(++i, 0, homeFile);
    qApp->processEvents();
    QTest::qWait(200);
    view.setCurrentIndex(two);
    view.scrollTo(two);
    qApp->processEvents();
    QTest::qWait(200);
    qApp->processEvents();
    QVERIFY(two.isValid());
    QVERIFY(view.HorizontalOffset() != 0);

    view.setRootIndex(homeFile);
    QCOMPARE(view.HorizontalOffset(), 0);
}

void tst_QColumnView::grips()
{
    QColumnView view;
    QDirModel model;
    view.setModel(&model);
    QCOMPARE(view.resizeGripsVisible(), true);

    view.setResizeGripsVisible(true);
    QCOMPARE(view.resizeGripsVisible(), true);

    {
        const QObjectList list = view.viewport()->children();
        for (int i = 0 ; i < list.count(); ++i) {
            if (QAbstractItemView *view = qobject_cast<QAbstractItemView*>(list.at(i)))
                QVERIFY(view->cornerWidget() != 0);
        }
    }
    view.setResizeGripsVisible(false);
    QCOMPARE(view.resizeGripsVisible(), false);

    {
        const QObjectList list = view.viewport()->children();
        for (int i = 0 ; i < list.count(); ++i) {
            if (QAbstractItemView *view = qobject_cast<QAbstractItemView*>(list.at(i))) {
                if (view->isVisible())
                    QVERIFY(view->cornerWidget() == 0);
            }
        }
    }

    view.setResizeGripsVisible(true);
    QCOMPARE(view.resizeGripsVisible(), true);
}

void tst_QColumnView::isIndexHidden()
{
    ColumnView view;
    QModelIndex idx;
    QCOMPARE(view.IsIndexHidden(idx), false);
    QDirModel model;
    view.setModel(&model);
    QCOMPARE(view.IsIndexHidden(idx), false);
}

void tst_QColumnView::indexAt()
{
    QColumnView view;
    QCOMPARE(view.indexAt(QPoint(0,0)), QModelIndex());
    QDirModel model;
    view.setModel(&model);

    QModelIndex home = model.index(QDir::homePath());
    QModelIndex homeFile = model.index(0, 0, home);
    if (!homeFile.isValid())
        return;
    view.setRootIndex(home);
    QRect rect = view.visualRect(QModelIndex());
    QVERIFY(!rect.isValid());
    rect = view.visualRect(homeFile);
    QVERIFY(rect.isValid());

    QModelIndex child;
    for (int i = 0; i < model.rowCount(home); ++i) {
        child = model.index(i, 0, home);
        rect = view.visualRect(child);
        QVERIFY(rect.isValid());
        if (i > 0)
            QVERIFY(rect.top() > 0);
        QCOMPARE(view.indexAt(rect.center()), child);

        view.selectionModel()->select(child, QItemSelectionModel::SelectCurrent);
        view.setCurrentIndex(child);
        qApp->processEvents();
        QTest::qWait(200);

        // test that the second row doesn't start at 0
        if (model.rowCount(child) > 0) {
            child = model.index(0, 0, child);
            QVERIFY(child.isValid());
            rect = view.visualRect(child);
            QVERIFY(rect.isValid());
            QVERIFY(rect.left() > 0);
            QCOMPARE(view.indexAt(rect.center()), child);
            break;
        }
    }
}

void tst_QColumnView::scrollContentsBy_data()
{
    QTest::addColumn<bool>("reverse");
    QTest::newRow("normal") << false;
    QTest::newRow("reverse") << true;
}


void tst_QColumnView::scrollContentsBy()
{
    QFETCH(bool, reverse);
    if (reverse)
        qApp->setLayoutDirection(Qt::RightToLeft);
    ColumnView view;
    view.ScrollContentsBy(-1, -1);
    view.ScrollContentsBy(0, 0);
    // ### view.children?
}

void tst_QColumnView::scrollTo()
{
    ColumnView view;
    view.resize(200, 200);
    view.show();
    view.scrollTo(QModelIndex(), QAbstractItemView::EnsureVisible);
    QCOMPARE(view.HorizontalOffset(), 0);

    QDirModel model;
    view.setModel(&model);
    view.scrollTo(QModelIndex(), QAbstractItemView::EnsureVisible);

    QModelIndex home = model.index(QDir::currentPath()).parent();
    QModelIndex homeFile = model.index(0, 0, home);
    view.setRootIndex(home);

    QModelIndex index = model.index(0, 0, home);
    view.scrollTo(index, QAbstractItemView::EnsureVisible);
    QCOMPARE(view.HorizontalOffset(), 0);

    view.clearFocus();
    QCOMPARE(view.hasFocus(), false);
    // scroll to the right
    int level = 0;
    int last = view.HorizontalOffset();
    while(model.hasChildren(index) && level < 8) {
        view.setCurrentIndex(index);
        QTest::qWait(ANIMATION_DELAY);
        view.scrollTo(index, QAbstractItemView::EnsureVisible);
        QTest::qWait(ANIMATION_DELAY);
        index = model.index(0, 0, index);
        level++;
        if (level >= 2) {
            QVERIFY(view.HorizontalOffset() < 0);
            QVERIFY(last > view.HorizontalOffset());
        }
        last = view.HorizontalOffset();
    }
    // It shouldn't automatically steal focus
    QCOMPARE(view.hasFocus(), false);

    // scroll to the left
    int start = level;
    while(index.parent().isValid() && index != view.rootIndex()) {
        view.setCurrentIndex(index);
        QTest::qWait(ANIMATION_DELAY);
        view.scrollTo(index, QAbstractItemView::EnsureVisible);
        index = index.parent();
        if (start != level)
            QVERIFY(last < view.HorizontalOffset());
        level--;
        last = view.HorizontalOffset();
    }

    // Try scrolling to something that is above the root index
    home = model.index(QDir::homePath());
    QModelIndex temp = model.index(QDir::tempPath());
    view.setRootIndex(home);
    view.scrollTo(model.index(0, 0, home));
    QTest::qWait(ANIMATION_DELAY);
    view.scrollTo(temp);
}

void tst_QColumnView::moveCursor_data()
{
    QTest::addColumn<bool>("reverse");
    QTest::newRow("normal") << false;
    QTest::newRow("reverse") << true;
}

void tst_QColumnView::moveCursor()
{
    QFETCH(bool, reverse);
    if (reverse)
        qApp->setLayoutDirection(Qt::RightToLeft);
    ColumnView view;

    // don't crash
    view.MoveCursor(ColumnView::MoveUp, Qt::NoModifier);

    // don't do anything
    QCOMPARE(view.MoveCursor(ColumnView::MoveEnd, Qt::NoModifier), QModelIndex());

    QDirModel model;
    view.setModel(&model);
    QModelIndex home = model.index(QDir::homePath());
    QModelIndex ci = view.currentIndex();
    QCOMPARE(view.MoveCursor(ColumnView::MoveUp, Qt::NoModifier), QModelIndex());
    QCOMPARE(view.MoveCursor(ColumnView::MoveDown, Qt::NoModifier), QModelIndex());

    // left at root
    view.setCurrentIndex(model.index(0,0));
    ColumnView::PublicCursorAction action = reverse ? ColumnView::MoveRight : ColumnView::MoveLeft;
    QCOMPARE(view.MoveCursor(action, Qt::NoModifier), model.index(0,0));

    // left shouldn't move up
    int i = 0;
    ci = model.index(0, 0);
    while (i < model.rowCount() - 1 && !model.hasChildren(ci))
        ci = model.index(++i, 0);
    QVERIFY(model.hasChildren(ci));
    view.setCurrentIndex(ci);
    action = reverse ? ColumnView::MoveRight : ColumnView::MoveLeft;
    QCOMPARE(view.MoveCursor(action, Qt::NoModifier), ci);

    // now move to the left (i.e. move over one column)
    view.setCurrentIndex(home);
    QCOMPARE(view.MoveCursor(action, Qt::NoModifier), home.parent());

    // right
    action = reverse ? ColumnView::MoveLeft : ColumnView::MoveRight;
    view.setCurrentIndex(ci);
    QModelIndex mc = view.MoveCursor(action, Qt::NoModifier);
    QCOMPARE(mc, model.index(0,0, ci));

    // next one should move down
    QModelIndex idx = model.index(0, 0, ci);
    while (model.hasChildren(idx) && model.rowCount(ci) > idx.row() + 1)
        idx = idx.sibling(idx.row() + 1, idx.column());
    view.setCurrentIndex(idx);
    mc = view.MoveCursor(action, Qt::NoModifier);
    QCOMPARE(mc, idx.sibling(idx.row() + 1, idx.column()));
}

void tst_QColumnView::selectAll()
{
    ColumnView view;
    view.selectAll();

    QDirModel model;
    view.setModel(&model);
    view.selectAll();
    QVERIFY(view.selectionModel()->selectedIndexes().count() >= 0);

    QModelIndex home = model.index(QDir::homePath());
    view.setCurrentIndex(home);
    view.selectAll();
    QVERIFY(view.selectionModel()->selectedIndexes().count() > 0);

    QModelIndex file;
    for (int i = 0; i < model.rowCount(home); ++i)
        if (!model.hasChildren(model.index(i, 0, home))) {
            file = model.index(i, 0, home);
            break;
        }
    view.setCurrentIndex(file);
    view.selectAll();
    QVERIFY(view.selectionModel()->selectedIndexes().count() > 0);

    view.setCurrentIndex(QModelIndex());
    QVERIFY(view.selectionModel()->selectedIndexes().count() == 0);
}

void tst_QColumnView::clicked()
{
    ColumnView view;
    QDirModel model;
    view.setModel(&model);
    view.resize(800,300);
    view.show();

    QModelIndex home = model.index(QDir::homePath());
    QVERIFY(home.isValid());
    view.setCurrentIndex(home);
    QTest::qWait(ANIMATION_DELAY);

    QModelIndex parent = home.parent();
    QVERIFY(parent.isValid());

    //child = child.sibling(child.row()-1, 0);

    qRegisterMetaType<QModelIndex>("QModelIndex");
    QSignalSpy spy(&view, SIGNAL(clicked(const QModelIndex &)));

    // find the column to click on that contains child
    QRect rect = view.visualRect(home);
    QVERIFY(!rect.isNull());
    TRY_VERIFY(view.isVisible());
    QPoint globalPoint = view.mapToGlobal(rect.center());
    QVERIFY(!globalPoint.isNull());
    QWidget *w = QApplication::widgetAt(globalPoint);
    QVERIFY(QApplication::topLevelAt(globalPoint));
    QVERIFY(w);
    QPoint localPoint = w->mapFromGlobal(globalPoint);

    QTest::mouseClick(w, Qt::LeftButton, 0, localPoint);
    QCOMPARE(spy.count(), 1);
    qApp->processEvents();
    for (int i = 0; i < view.createdColumns.count(); ++i) {
        QAbstractItemView *column = view.createdColumns.at(i);
        if (column && column->selectionModel() && (column->rootIndex() == home))
                QVERIFY(column->selectionModel()->selectedIndexes().isEmpty());
    }
}

void tst_QColumnView::selectedColumns()
{
    ColumnView view;
    QDirModel model;
    view.setModel(&model);
    view.resize(800,300);
    view.show();

    QModelIndex home = model.index(QDir::homePath());
    view.setCurrentIndex(home);

    QTest::qWait(ANIMATION_DELAY);

    for (int i = 0; i < view.createdColumns.count(); ++i) {
        QAbstractItemView *column = view.createdColumns.at(i);
        if (!column)
            continue;
        if (!column->rootIndex().isValid() || column->rootIndex() == home)
            continue;
        QVERIFY(column->currentIndex().isValid());
    }
}

void tst_QColumnView::setSelection()
{
    ColumnView view;
    // shouldn't do anything, it falls to the columns to handle this
    QRect r;
    view.setSelection(r, QItemSelectionModel::NoUpdate);
}

void tst_QColumnView::setSelectionModel()
{
    ColumnView view;
    QDirModel model;
    view.setModel(&model);
    view.show();

    QModelIndex home = model.index(QDir::homePath());
    view.setCurrentIndex(home);
    QTest::qWait(ANIMATION_DELAY);

    QItemSelectionModel *selectionModel = new QItemSelectionModel(&model);
    view.setSelectionModel(selectionModel);

    bool found = false;
    for (int i = 0; i < view.createdColumns.count(); ++i) {
        if (view.createdColumns.at(i)->selectionModel() == selectionModel) {
            found = true;
            break;
        }
    }
    QVERIFY(found);
}

void tst_QColumnView::visualRegionForSelection()
{
    ColumnView view;
    QItemSelection emptyItemSelection;
    QCOMPARE(QRegion(), view.visualRegionForSelection(emptyItemSelection));

    // a region that isn't empty
    QDirModel model;
    view.setModel(&model);

    QModelIndex home = model.index(QDir::homePath());
    QVERIFY(model.rowCount(home) > 1);
    QItemSelection itemSelection(model.index(0, 0, home), model.index(model.rowCount(home) - 1, 0, home));
    QVERIFY(QRegion() != view.visualRegionForSelection(itemSelection));
}

void tst_QColumnView::moveGrip_basic()
{
    QColumnView view;
    QColumnViewGrip *grip = new QColumnViewGrip(&view);
    QSignalSpy spy(grip, SIGNAL(gripMoved(int)));
    view.setCornerWidget(grip);
    int oldX = view.width();
    grip->moveGrip(10);
    QCOMPARE(oldX + 10, view.width());
    grip->moveGrip(-10);
    QCOMPARE(oldX, view.width());
    grip->moveGrip(-800);
    QVERIFY(view.width() == 0 || view.width() == 1);
    grip->moveGrip(800);
    view.setMinimumWidth(200);
    grip->moveGrip(-800);
    QCOMPARE(view.width(), 200);
    QCOMPARE(spy.count(), 5);
}

void tst_QColumnView::moveGrip_data()
{
    QTest::addColumn<bool>("reverse");
    QTest::newRow("normal") << false;
    QTest::newRow("reverse") << true;
}

void tst_QColumnView::moveGrip()
{
    QFETCH(bool, reverse);
    if (reverse)
        qApp->setLayoutDirection(Qt::RightToLeft);
    ColumnView view;
    QDirModel model;
    view.setModel(&model);
    QModelIndex home = model.index(QDir::homePath());
    view.setCurrentIndex(home);
    view.resize(640, 200);
    view.show();
    QTest::qWait(ANIMATION_DELAY);

    int columnNum = view.createdColumns.count() - 2;
    QVERIFY(columnNum >= 0);
    QObjectList list = view.createdColumns[columnNum]->children();
    QColumnViewGrip *grip = 0;
    for (int i = 0; i < list.count(); ++i) {
        if ((grip = qobject_cast<QColumnViewGrip *>(list[i]))) {
            break;
        }
    }
    if (!grip)
        return;

    QAbstractItemView *column = qobject_cast<QAbstractItemView *>(grip->parent());
    int oldX = column->width();
    QCOMPARE(view.columnWidths()[columnNum], oldX);
    grip->moveGrip(10);
    QCOMPARE(view.columnWidths()[columnNum], (oldX + (reverse ? -10 : 10)));
}

void tst_QColumnView::doubleClick()
{
    QColumnView view;
    QColumnViewGrip *grip = new QColumnViewGrip(&view);
    QSignalSpy spy(grip, SIGNAL(gripMoved(int)));
    view.setCornerWidget(grip);
    view.resize(200, 200);
    QCOMPARE(view.width(), 200);
    QTest::mouseDClick(grip, Qt::LeftButton);
    QCOMPARE(view.width(), view.sizeHint().width());
    QCOMPARE(spy.count(), 1);
}

void tst_QColumnView::gripMoved()
{
    QColumnView view;
    QColumnViewGrip *grip = new QColumnViewGrip(&view);
    QSignalSpy spy(grip, SIGNAL(gripMoved(int)));
    view.setCornerWidget(grip);
    view.move(300, 300);
    view.resize(200, 200);
    qApp->processEvents();

    int oldWidth = view.width();

    QTest::mousePress(grip, Qt::LeftButton, 0, QPoint(1,1));
    //QTest::mouseMove(grip, QPoint(grip->globalX()+50, y));

    QPoint posNew = QPoint(grip->mapToGlobal(QPoint(1,1)).x() + 65, 0);
    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove, posNew, posNew, Qt::LeftButton, Qt::LeftButton,Qt::NoModifier);
    QCoreApplication::postEvent(grip, event);
    QCoreApplication::processEvents();
    QTest::mouseRelease(grip, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(view.width(), oldWidth + 65);
}

void tst_QColumnView::preview()
{
    QColumnView view;
    QCOMPARE(view.previewWidget(), (QWidget*)0);
    QDirModel model;
    view.setModel(&model);
    QCOMPARE(view.previewWidget(), (QWidget*)0);
    QModelIndex home = model.index(QDir::homePath());
    view.setCurrentIndex(home);
    QCOMPARE(view.previewWidget(), (QWidget*)0);

    QModelIndex file;
    for (int i = 0; i < model.rowCount(home); ++i)
        if (!model.hasChildren(model.index(i, 0, home))) {
            file = model.index(i, 0, home);
            break;
        }
    view.setCurrentIndex(file);
    QVERIFY(view.previewWidget() != (QWidget*)0);

    QWidget *previewWidget = new QWidget(&view);
    view.setPreviewWidget(previewWidget);
    QCOMPARE(view.previewWidget(), previewWidget);
    QVERIFY(previewWidget->parent() != ((QWidget*)&view));
    view.setCurrentIndex(home);

    // previewWidget should be marked for deletion
    QWidget *previewWidget2 = new QWidget(&view);
    view.setPreviewWidget(previewWidget2);
    QCOMPARE(view.previewWidget(), previewWidget2);
}

void tst_QColumnView::swapPreview()
{
    // swap the preview widget in updatePreviewWidget
    QColumnView view;
    QStringList sl;
    sl << QLatin1String("test");
    QStringListModel model(sl);
    view.setModel(&model);
    view.setCurrentIndex(view.indexAt(QPoint(1, 1)));
    connect(&view, SIGNAL(updatePreviewWidget(const QModelIndex &)),
            this, SLOT(setPreviewWidget()));
    view.setCurrentIndex(view.indexAt(QPoint(1, 1)));
    QTest::qWait(ANIMATION_DELAY);
    qApp->processEvents();
}

void tst_QColumnView::setPreviewWidget()
{
    ((QColumnView*)sender())->setPreviewWidget(new QWidget);
}

void tst_QColumnView::sizes()
{
    QColumnView view;
    QCOMPARE(view.columnWidths().count(), 0);

    QList<int> newSizes;
    newSizes << 10 << 4 << 50 << 6;

    QList<int> visibleSizes;
    view.setColumnWidths(newSizes);
    QCOMPARE(view.columnWidths(), visibleSizes);

    QDirModel model;
    view.setModel(&model);
    QModelIndex home = model.index(QDir::homePath());
    view.setCurrentIndex(home);

    QList<int> postSizes = view.columnWidths().mid(0, newSizes.count());
    QCOMPARE(postSizes, newSizes.mid(0, postSizes.count()));

    QList<int> smallerSizes;
    smallerSizes << 6;
    view.setColumnWidths(smallerSizes);
    QList<int> expectedSizes = newSizes;
    expectedSizes[0] = 6;
    postSizes = view.columnWidths().mid(0, newSizes.count());
    QCOMPARE(postSizes, expectedSizes.mid(0, postSizes.count()));
}

void tst_QColumnView::rowDelegate()
{
    ColumnView view;
    QItemDelegate *d = new QItemDelegate;
    view.setItemDelegateForRow(3, d);

    QDirModel model;
    view.setModel(&model);
    for (int i = 0; i < view.createdColumns.count(); ++i) {
        QAbstractItemView *column = view.createdColumns.at(i);
        QCOMPARE(column->itemDelegateForRow(3), (QAbstractItemDelegate*)d);
    }
    delete d;
}

void tst_QColumnView::resize()
{
    ColumnView view;
    QDirModel model;
    view.setModel(&model);
    view.resize(200, 200);

    view.show();
    QModelIndex home = model.index(QDir::homePath()).parent();
    view.setCurrentIndex(home);
    QTest::qWait(ANIMATION_DELAY);

    QVERIFY(view.horizontalScrollBar()->maximum() != 0);
    view.resize(view.horizontalScrollBar()->maximum() * 10, 200);
    QTest::qWait(ANIMATION_DELAY);
    QVERIFY(view.horizontalScrollBar()->maximum() <= 0);
}

void tst_QColumnView::changeSameColumn()
{
    ColumnView view;
    QDirModel model;
    view.setModel(&model);
    QModelIndex second;

    QModelIndex home = model.index(QDir::homePath());
    view.setCurrentIndex(home);
    for (int i = 0; i < model.rowCount(home.parent()); ++i) {
        QModelIndex idx = model.index(i, 0, home.parent());
        if (model.hasChildren(idx) && idx != home) {
            second = idx;
            break;
        }
    }
    if (!second.isValid()) {
        home = model.index(QDir::tempPath());
        view.setCurrentIndex(home);
        for (int i = 0; i < model.rowCount(home.parent()); ++i) {
            QModelIndex idx = model.index(i, 0, home.parent());
            if (model.hasChildren(idx) && idx != home) {
                second = idx;
                break;
            }
        }
    }
    QVERIFY(second.isValid());

    QList<QPointer<QAbstractItemView> > old = view.createdColumns;
    view.setCurrentIndex(second);

    QCOMPARE(old, view.createdColumns);
}

QTEST_MAIN(tst_QColumnView)
#include "tst_qcolumnview.moc"

