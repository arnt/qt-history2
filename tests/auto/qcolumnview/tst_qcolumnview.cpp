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
#include <QStandardItemModel>
#include <qcolumnview.h>
#include "../../../src/gui/itemviews/qcolumnviewgrip_p.h"
#include <qdirmodel.h>
#include <qdebug.h>

//TESTED_CLASS=QColumnView
//TESTED_FILES=qcolumnview.h qcolumnview.cpp

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
    void scrollContentsBy();
    void scrollTo();
    void moveCursor();

    // grip
    void moveGrip();
    void doubleClick();
    void gripMoved();

protected:
};

class ColumnView : public QColumnView {
    public:
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
    bool IsIndexHidden(const QModelIndex&index) const { return isIndexHidden(index); }
};

tst_QColumnView::tst_QColumnView()
{
}

tst_QColumnView::~tst_QColumnView()
{
}

void tst_QColumnView::init()
{
}

void tst_QColumnView::cleanup()
{
}

void tst_QColumnView::rootIndex()
{
    QColumnView view;
    // no model
    view.setRootIndex(QModelIndex());

    QDirModel model;
    view.setModel(&model);

    QModelIndex drive = model.index(0, 0);
    QVERIFY(view.visualRect(drive).isValid());
    view.setRootIndex(QModelIndex());
    QCOMPARE(view.rootIndex(), QModelIndex());
    QVERIFY(view.visualRect(drive).isValid());

    QModelIndex home = model.index(QDir::homePath());
    QModelIndex homeFile = model.index(0, 0, home);
    view.setRootIndex(home);
    QCOMPARE(view.rootIndex(), home);
    QVERIFY(!view.visualRect(drive).isValid());
    QVERIFY(!view.visualRect(home).isValid());
    if (homeFile.isValid())
        QVERIFY(view.visualRect(homeFile).isValid());

    // set root when there already is one
    view.setRootIndex(home);
    QCOMPARE(view.rootIndex(), home);
    QVERIFY(!view.visualRect(drive).isValid());
    QVERIFY(!view.visualRect(home).isValid());
     if (homeFile.isValid())
        QVERIFY(view.visualRect(homeFile).isValid());
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

void tst_QColumnView::scrollContentsBy()
{
    ColumnView view;
    view.ScrollContentsBy(-1, -1);
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

    QModelIndex index = model.index(0, 0);
    view.scrollTo(index, QAbstractItemView::EnsureVisible);
    QCOMPARE(view.HorizontalOffset(), 0);

    // scroll to the right
    int level = 0;
    int last = view.HorizontalOffset();
    while(model.hasChildren(index) && level < 8) {
        view.setCurrentIndex(index);
        QTest::qWait(300);
        view.scrollTo(index, QAbstractItemView::EnsureVisible);
        QTest::qWait(300);
        index = model.index(0, 0, index);
        level++;
        if (level >= 2) {
            QVERIFY(view.HorizontalOffset() < 0);
            QVERIFY(last > view.HorizontalOffset());
        }
        last = view.HorizontalOffset();
    }

    // scroll to the left
    int start = level;
    while(index.parent().isValid()) {
        view.setCurrentIndex(index);
        QTest::qWait(300);
        view.scrollTo(index, QAbstractItemView::EnsureVisible);
        index = index.parent();
        if (start != level)
            QVERIFY(last < view.HorizontalOffset());
        level--;
        last = view.HorizontalOffset();
    }

}

void tst_QColumnView::moveCursor()
{
    ColumnView view;
    QDirModel model;
    view.setModel(&model);
    QModelIndex ci = view.currentIndex();
    QCOMPARE(view.MoveCursor(ColumnView::MoveUp, Qt::NoModifier), QModelIndex());
    QCOMPARE(view.MoveCursor(ColumnView::MoveDown, Qt::NoModifier), QModelIndex());

    int i = 0;
    ci = model.index(0, 0);
    while (i < model.rowCount() - 1 && !model.hasChildren(ci))
        ci = model.index(++i, 0);

    QVERIFY(model.hasChildren(ci));

    view.setCurrentIndex(ci);
    QCOMPARE(view.MoveCursor(ColumnView::MoveLeft, Qt::NoModifier), ci);

    view.setCurrentIndex(ci);
    QCOMPARE(view.MoveCursor(ColumnView::MoveRight, Qt::NoModifier), model.index(0,0, ci));
}

void tst_QColumnView::moveGrip()
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
    QCOMPARE(view.width(), 0);
    grip->moveGrip(800);
    view.setMinimumWidth(200);
    grip->moveGrip(-800);
    QCOMPARE(view.width(), 200);
    QCOMPARE(spy.count(), 5);
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

QTEST_MAIN(tst_QColumnView)
#include "tst_qcolumnview.moc"

