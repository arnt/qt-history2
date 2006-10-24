/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#if QT_VERSION < 0x040200
QTEST_NOOP_MAIN
#else

#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsview.h>

#include <math.h>

#include <QtGui/QLabel>
#include <QtGui/QPainterPath>
#include <QtGui/QRubberBand>
#include <QtGui/QScrollBar>

//TESTED_CLASS=QGraphicsView
//TESTED_FILES=gui/graphicsview/qgraphicsview.cpp

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<QRectF>)
Q_DECLARE_METATYPE(QMatrix)
Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPointF)
Q_DECLARE_METATYPE(QRectF)

static void sendMouseMove(QWidget *widget, const QPoint &point)
{
    QMouseEvent event(QEvent::MouseMove, point, Qt::NoButton, 0, 0);
    QApplication::sendEvent(widget, &event);
}

class tst_QGraphicsView : public QObject
{
    Q_OBJECT

private slots:
    void construction();
    void renderHints();
    void alignment();
    void interactive();
    void scene();
    void sceneRect();
    void sceneRect_growing();
    void setSceneRect();
    void viewport();
    void dragMode_scrollHand();
    void dragMode_rubberBand();
    void backgroundBrush();
    void foregroundBrush();
    void matrix();
    void matrix_convenience();
    void matrix_combine();
    void centerOnPoint();
    void centerOnItem();
    void ensureVisibleRect();
    void fitInView();
    void itemsAtPoint();
    void itemsInRect();
    void itemsInPoly();
    void itemsInPath();
    void itemAt();
    void mapToScene();
    void mapToScenePoint();
    void mapToSceneRect();
    void mapToScenePoly();
    void mapToScenePath();
    void mapFromScenePoint();
    void mapFromSceneRect();
    void mapFromScenePoly();
    void mapFromScenePath();
    void sendEvent();
    void cursor();
    void cursor2();
    void transformationAnchor();
    void resizeAnchor();
};

void tst_QGraphicsView::construction()
{
    QGraphicsView view;
    QCOMPARE(view.renderHints(), QPainter::TextAntialiasing);
    QCOMPARE(view.dragMode(), QGraphicsView::NoDrag);
    QVERIFY(view.isInteractive());
    QVERIFY(!view.scene());
    QCOMPARE(view.sceneRect(), QRectF());
    QVERIFY(view.viewport());
    QCOMPARE(view.viewport()->metaObject()->className(), "QWidget");
    QCOMPARE(view.matrix(), QMatrix());
    QVERIFY(view.items().isEmpty());
    QVERIFY(view.items(QPoint()).isEmpty());
    QVERIFY(view.items(QRect()).isEmpty());
    QVERIFY(view.items(QPolygon()).isEmpty());
    QVERIFY(view.items(QPainterPath()).isEmpty());
    QVERIFY(!view.itemAt(QPoint()));
    QCOMPARE(view.mapToScene(QPoint()), QPointF());
    QCOMPARE(view.mapToScene(QRect()), QPolygonF());
    QCOMPARE(view.mapToScene(QPolygon()), QPolygonF());
    QCOMPARE(view.mapFromScene(QPointF()), QPoint());
    QCOMPARE(view.mapFromScene(QRectF()), QPolygon());
    QCOMPARE(view.mapFromScene(QPolygonF()), QPolygon());
    QCOMPARE(view.transformationAnchor(), QGraphicsView::AnchorViewCenter);
    QCOMPARE(view.resizeAnchor(), QGraphicsView::NoAnchor);
    view.show();
    QTest::qWait(25);
}

class TestItem : public QGraphicsItem
{
public:
    QRectF boundingRect() const
    { return QRectF(-10, -10, 10, 10); }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    { hints = painter->renderHints(); painter->drawRect(boundingRect()); }

    bool sceneEvent(QEvent *event)
    {
        events << event->type();
        return QGraphicsItem::sceneEvent(event);
    }

    QList<QEvent::Type> events;
    QPainter::RenderHints hints;
};

void tst_QGraphicsView::renderHints()
{
    QGraphicsView view;
    QCOMPARE(view.renderHints(), QPainter::TextAntialiasing);
    view.setRenderHint(QPainter::TextAntialiasing, false);
    QCOMPARE(view.renderHints(), 0);
    view.setRenderHint(QPainter::Antialiasing, false);
    QCOMPARE(view.renderHints(), 0);
    view.setRenderHint(QPainter::TextAntialiasing, true);
    QCOMPARE(view.renderHints(), QPainter::TextAntialiasing);
    view.setRenderHint(QPainter::Antialiasing);
    QCOMPARE(view.renderHints(), QPainter::TextAntialiasing | QPainter::Antialiasing);
    view.setRenderHints(0);
    QCOMPARE(view.renderHints(), 0);

    view.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    QCOMPARE(view.renderHints(), QPainter::TextAntialiasing | QPainter::Antialiasing);

    TestItem *item = new TestItem;
    QGraphicsScene scene;
    scene.addItem(item);

    view.setScene(&scene);

    QCOMPARE(item->hints, 0);
    view.show();
    view.repaint();
    QTest::qWait(25);
    QCOMPARE(item->hints, view.renderHints());
}

void tst_QGraphicsView::alignment()
{
    QGraphicsScene scene;
    scene.addRect(QRectF(-10, -10, 20, 20));

    QGraphicsView view(&scene);
    view.show();

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Qt::Alignment alignment = 0;
            switch (i) {
            case 0:
                alignment |= Qt::AlignLeft;
                break;
            case 1:
                alignment |= Qt::AlignHCenter;
                break;
            case 2:
            default:
                alignment |= Qt::AlignRight;
                break;
            }
            switch (j) {
            case 0:
                alignment |= Qt::AlignTop;
                break;
            case 1:
                alignment |= Qt::AlignVCenter;
                break;
            case 2:
            default:
                alignment |= Qt::AlignBottom;
                break;
            }
            view.setAlignment(alignment);
            QCOMPARE(view.alignment(), alignment);

            for (int k = 0; k < 3; ++k) {
                view.resize(100 + k * 25, 100 + k * 25);
                QTest::qWait(25);
            }
        }
    }
}

void tst_QGraphicsView::interactive()
{
    TestItem *item = new TestItem;
    QCOMPARE(item->events.size(), 0);

    QGraphicsScene scene;
    scene.addItem(item);

    QGraphicsView view(&scene);
    QCOMPARE(item->events.size(), 0);
    view.show();
    view.repaint();
    QCOMPARE(item->events.size(), 0);

    for (int i = 0; i < 100; ++i) {
        QMouseEvent pressEvent(QEvent::MouseButtonPress, view.viewport()->rect().center(),
                               Qt::LeftButton, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &pressEvent);
        QCOMPARE(item->events.size(), i * 3 + 1);
        QCOMPARE(item->events.last(), QEvent::GraphicsSceneMousePress);
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, view.viewport()->rect().center(),
                                 Qt::LeftButton, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &releaseEvent);
        QCOMPARE(item->events.size(), i * 3 + 2);
        QCOMPARE(item->events.last(), QEvent::GraphicsSceneMouseRelease);
        QContextMenuEvent contextEvent(QContextMenuEvent::Mouse, view.viewport()->rect().center());
        QApplication::sendEvent(view.viewport(), &contextEvent);
        QCOMPARE(item->events.size(), i * 3 + 3);
        QCOMPARE(item->events.last(), QEvent::GraphicsSceneContextMenu);
    }

    view.setInteractive(false);

    for (int i = 0; i < 100; ++i) {
        QMouseEvent pressEvent(QEvent::MouseButtonPress, view.viewport()->rect().center(),
                               Qt::LeftButton, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &pressEvent);
        QCOMPARE(item->events.size(), 300);
        QCOMPARE(item->events.last(), QEvent::GraphicsSceneContextMenu);
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, view.viewport()->rect().center(),
                                 Qt::LeftButton, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &releaseEvent);
        QCOMPARE(item->events.size(), 300);
        QCOMPARE(item->events.last(), QEvent::GraphicsSceneContextMenu);
        QContextMenuEvent contextEvent(QContextMenuEvent::Mouse, view.viewport()->rect().center());
        QApplication::sendEvent(view.viewport(), &contextEvent);
        QCOMPARE(item->events.size(), 300);
        QCOMPARE(item->events.last(), QEvent::GraphicsSceneContextMenu);
    }
}

void tst_QGraphicsView::scene()
{
    QGraphicsView view;
    QVERIFY(!view.scene());
    view.setScene(0);
    QVERIFY(!view.scene());

    QGraphicsScene scene;
    view.setScene(&scene);
    QCOMPARE(view.scene(), &scene);
}

void tst_QGraphicsView::sceneRect()
{
    QGraphicsView view;
    QCOMPARE(view.sceneRect(), QRectF());

    view.setSceneRect(QRectF(-100, -100, 200, 200));
    QCOMPARE(view.sceneRect(), QRectF(-100, -100, 200, 200));
    view.setSceneRect(-100, -100, 200, 200);
    QCOMPARE(view.sceneRect(), QRectF(-100, -100, 200, 200));

    view.setSceneRect(QRectF());
    QCOMPARE(view.sceneRect(), QRectF());
    QGraphicsScene scene;
    QGraphicsRectItem *item = scene.addRect(QRectF(-100, -100, 100, 100));

    view.setScene(&scene);

    QCOMPARE(view.sceneRect(), QRectF(-100, -100, 100, 100));
    item->moveBy(-100, -100);
    QCOMPARE(view.sceneRect(), QRectF(-200, -200, 200, 200));
    item->moveBy(100, 100);
    QCOMPARE(view.sceneRect(), QRectF(-200, -200, 200, 200));

    view.setScene(0);
    view.setSceneRect(QRectF());
    QCOMPARE(view.sceneRect(), QRectF());
}

void tst_QGraphicsView::sceneRect_growing()
{
    QGraphicsScene scene;
    for (int i = 0; i < 100; ++i)
        scene.addText(QString("(0, %1)").arg((i - 50) * 20))->setPos(0, (i - 50) * 20);

    QGraphicsView view(&scene);
    view.setFixedSize(200, 200);
    view.show();

    int size = 200;
    scene.setSceneRect(-size, -size, size * 2, size * 2);
    QCOMPARE(view.sceneRect(), scene.sceneRect());

    QTest::qWait(25);

    QPointF topLeft = view.mapToScene(0, 0);

    for (int i = 0; i < 5; ++i) {
        size *= 2;
        scene.setSceneRect(-size, -size, size * 2, size * 2);

        QTest::qWait(25);

        QCOMPARE(view.sceneRect(), scene.sceneRect());
        QCOMPARE(view.mapToScene(0, 0), topLeft);
        view.setSceneRect(-size, -size, size * 2, size * 2);
        QCOMPARE(view.mapToScene(0, 0), topLeft);
        view.setSceneRect(QRectF());
    }
}

void tst_QGraphicsView::setSceneRect()
{
    QRectF rect1(-100, -100, 200, 200);
    QRectF rect2(-300, -300, 150, 150);

    QGraphicsScene scene;
    QGraphicsView view(&scene);

    scene.setSceneRect(rect1);
    QCOMPARE(scene.sceneRect(), rect1);
    QCOMPARE(view.sceneRect(), rect1);

    scene.setSceneRect(rect2);
    QCOMPARE(scene.sceneRect(), rect2);
    QCOMPARE(view.sceneRect(), rect2);

    view.setSceneRect(rect1);
    QCOMPARE(scene.sceneRect(), rect2);
    QCOMPARE(view.sceneRect(), rect1);

    view.setSceneRect(rect2);
    QCOMPARE(scene.sceneRect(), rect2);
    QCOMPARE(view.sceneRect(), rect2);

    scene.setSceneRect(rect1);
    QCOMPARE(scene.sceneRect(), rect1);
    QCOMPARE(view.sceneRect(), rect2);
}

void tst_QGraphicsView::viewport()
{
    QGraphicsScene scene;
    scene.addText("GraphicsView");

    QGraphicsView view(&scene);
    QVERIFY(view.viewport() != 0);

    view.show();
    QTest::qWait(25);

    QPointer<QWidget> widget = new QWidget;
    view.setViewport(widget);
    QCOMPARE(view.viewport(), (QWidget *)widget);

    view.show();
    QTest::qWait(25);

    view.setViewport(0);
    QVERIFY(widget.isNull());
    QVERIFY(view.viewport() != 0);
    QVERIFY(view.viewport() != widget);

    view.show();
    QTest::qWait(25);
}

void tst_QGraphicsView::dragMode_scrollHand()
{
    QGraphicsView view;
    QCOMPARE(view.dragMode(), QGraphicsView::NoDrag);

    view.setSceneRect(-1000, -1000, 2000, 2000);
    view.setFixedSize(100, 100);
    view.show();

    QTest::qWait(25);

    QGraphicsScene scene;
    scene.addRect(QRectF(-100, -100, 5, 5));
    scene.addRect(QRectF(95, -100, 5, 5));
    scene.addRect(QRectF(95, 95, 5, 5));
    scene.addRect(QRectF(-100, 95, 5, 5));

    view.setDragMode(QGraphicsView::ScrollHandDrag);

    for (int i = 0; i < 2; ++i) {
        // ScrollHandDrag
        Qt::CursorShape cursorShape = view.viewport()->cursor().shape();
        int horizontalScrollBarValue = view.horizontalScrollBar()->value();
        int verticalScrollBarValue = view.verticalScrollBar()->value();
        {
            // Press
            QMouseEvent event(QEvent::MouseButtonPress,
                              view.viewport()->rect().center(),
                              Qt::LeftButton, Qt::LeftButton, 0);
            QApplication::sendEvent(view.viewport(), &event);
        }
        QTest::qWait(250);
        QCOMPARE(view.viewport()->cursor().shape(), Qt::ClosedHandCursor);
        {
            // Move
            QMouseEvent event(QEvent::MouseMove,
                              view.viewport()->rect().center() + QPoint(10, 0),
                              Qt::LeftButton, Qt::LeftButton, 0);
            QApplication::sendEvent(view.viewport(), &event);
        }
        QCOMPARE(view.horizontalScrollBar()->value(), horizontalScrollBarValue - 10);
        QCOMPARE(view.verticalScrollBar()->value(), verticalScrollBarValue);
        {
            // Move
            QMouseEvent event(QEvent::MouseMove,
                              view.viewport()->rect().center() + QPoint(10, 10),
                              Qt::LeftButton, Qt::LeftButton, 0);
            QApplication::sendEvent(view.viewport(), &event);
        }
        QCOMPARE(view.horizontalScrollBar()->value(), horizontalScrollBarValue - 10);
        QCOMPARE(view.verticalScrollBar()->value(), verticalScrollBarValue - 10);
        {
            // Release
            QMouseEvent event(QEvent::MouseButtonRelease,
                              view.viewport()->rect().center() + QPoint(10, 10),
                              Qt::LeftButton, Qt::LeftButton, 0);
            QApplication::sendEvent(view.viewport(), &event);
        }
        QTest::qWait(250);
        QCOMPARE(view.horizontalScrollBar()->value(), horizontalScrollBarValue - 10);
        QCOMPARE(view.verticalScrollBar()->value(), verticalScrollBarValue - 10);
        QCOMPARE(view.viewport()->cursor().shape(), cursorShape);

        view.setScene(&scene);
    }
}

void tst_QGraphicsView::dragMode_rubberBand()
{
    QGraphicsView view;
    QCOMPARE(view.dragMode(), QGraphicsView::NoDrag);

    view.setSceneRect(-1000, -1000, 2000, 2000);
    view.show();

    QGraphicsScene scene;
    scene.addRect(QRectF(-100, -100, 25, 25))->setFlag(QGraphicsItem::ItemIsSelectable);
    scene.addRect(QRectF(75, -100, 25, 25))->setFlag(QGraphicsItem::ItemIsSelectable);
    scene.addRect(QRectF(75, 75, 25, 25))->setFlag(QGraphicsItem::ItemIsSelectable);
    scene.addRect(QRectF(-100, 75, 25, 25))->setFlag(QGraphicsItem::ItemIsSelectable);

    view.setDragMode(QGraphicsView::RubberBandDrag);

    for (int i = 0; i < 2; ++i) {
        // RubberBandDrag
        Qt::CursorShape cursorShape = view.viewport()->cursor().shape();
        int horizontalScrollBarValue = view.horizontalScrollBar()->value();
        int verticalScrollBarValue = view.verticalScrollBar()->value();
        {
            // Press
            QMouseEvent event(QEvent::MouseButtonPress,
                              view.viewport()->rect().center(),
                              Qt::LeftButton, Qt::LeftButton, 0);
            QApplication::sendEvent(view.viewport(), &event);
        }
        QCOMPARE(view.viewport()->cursor().shape(), cursorShape);

        QTest::qWait(25);

        {
            // Move
            QMouseEvent event(QEvent::MouseMove,
                              view.viewport()->rect().center() + QPoint(100, 0),
                              Qt::LeftButton, Qt::LeftButton, 0);
            QApplication::sendEvent(view.viewport(), &event);
        }
        QCOMPARE(view.horizontalScrollBar()->value(), horizontalScrollBarValue);
        QCOMPARE(view.verticalScrollBar()->value(), verticalScrollBarValue);
        QVERIFY(qFindChild<QRubberBand *>(&view));

        QTest::qWait(25);

        {
            // Move
            QMouseEvent event(QEvent::MouseMove,
                              view.viewport()->rect().center() + QPoint(100, 100),
                              Qt::LeftButton, Qt::LeftButton, 0);
            QApplication::sendEvent(view.viewport(), &event);
        }
        QCOMPARE(view.horizontalScrollBar()->value(), horizontalScrollBarValue);
        QCOMPARE(view.verticalScrollBar()->value(), verticalScrollBarValue);

        QTest::qWait(25);

        {
            // Release
            QMouseEvent event(QEvent::MouseButtonRelease,
                              view.viewport()->rect().center() + QPoint(100, 100),
                              Qt::LeftButton, Qt::LeftButton, 0);
            QApplication::sendEvent(view.viewport(), &event);
        }
        QCOMPARE(view.horizontalScrollBar()->value(), horizontalScrollBarValue);
        QCOMPARE(view.verticalScrollBar()->value(), verticalScrollBarValue);
        QCOMPARE(view.viewport()->cursor().shape(), cursorShape);

        QTest::qWait(25);

        if (view.scene())
            QCOMPARE(scene.selectedItems().size(), 1);

        view.setScene(&scene);
        view.centerOn(0, 0);
    }
}

void tst_QGraphicsView::backgroundBrush()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    scene.setBackgroundBrush(Qt::blue);
    QCOMPARE(scene.backgroundBrush(), QBrush(Qt::blue));

    view.show();
    QTest::qWait(25);

    scene.setBackgroundBrush(QBrush());
    QCOMPARE(scene.backgroundBrush(), QBrush());
    QTest::qWait(25);

    QRadialGradient gradient(0, 0, 10);
    gradient.setSpread(QGradient::RepeatSpread);
    scene.setBackgroundBrush(gradient);

    QCOMPARE(scene.backgroundBrush(), QBrush(gradient));
    QTest::qWait(25);
}

void tst_QGraphicsView::foregroundBrush()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    scene.setForegroundBrush(Qt::blue);
    QCOMPARE(scene.foregroundBrush(), QBrush(Qt::blue));

    view.show();
    QTest::qWait(25);

    scene.setForegroundBrush(QBrush());
    QCOMPARE(scene.foregroundBrush(), QBrush());
    QTest::qWait(25);

    QRadialGradient gradient(0, 0, 10);
    gradient.setSpread(QGradient::RepeatSpread);
    scene.setForegroundBrush(gradient);

    QCOMPARE(scene.foregroundBrush(), QBrush(gradient));
    QTest::qWait(25);

    for (int i = 0; i < 50; ++i) {
        QRadialGradient gradient(view.rect().center() + QPoint(int(sin(i / 2.0) * 10), int(cos(i / 2.0) * 10)), 10);
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(0.5, Qt::black);
        gradient.setColorAt(1, Qt::transparent);
        gradient.setSpread(QGradient::RepeatSpread);
        scene.setForegroundBrush(gradient);

        QRadialGradient gradient2(view.rect().center() + QPoint(int(sin(i / 1.7) * 10), int(cos(i / 1.7) * 10)), 10);
        gradient2.setColorAt(0, Qt::transparent);
        gradient2.setColorAt(0.5, Qt::black);
        gradient2.setColorAt(1, Qt::transparent);
        gradient2.setSpread(QGradient::RepeatSpread);
        scene.setBackgroundBrush(gradient2);

        QRadialGradient gradient3(view.rect().center() + QPoint(int(sin(i / 1.85) * 10), int(cos(i / 1.85) * 10)), 10);
        gradient3.setColorAt(0, Qt::transparent);
        gradient3.setColorAt(0.5, Qt::black);
        gradient3.setColorAt(1, Qt::transparent);
        gradient3.setSpread(QGradient::RepeatSpread);
        scene.setBackgroundBrush(gradient3);

        QApplication::processEvents();
    }

    view.setSceneRect(-1000, -1000, 2000, 2000);
    for (int i = -500; i < 500; i += 10) {
        view.centerOn(i, 0);
        QTest::qWait(10);
    }
    for (int i = -500; i < 500; i += 10) {
        view.centerOn(0, i);
        QTest::qWait(10);
    }
}

void tst_QGraphicsView::matrix()
{
    {
        QGraphicsScene scene;
        QGraphicsView view(&scene);
        view.show();

        // Show rendering of background with no scene
        for (int i = 0; i < 50; ++i) {
            view.rotate(5);
            QRadialGradient gradient(view.rect().center() + QPoint(int(sin(i / 2.0) * 10), int(cos(i / 2.0) * 10)), 10);
            gradient.setColorAt(0, Qt::transparent);
            gradient.setColorAt(0.5, Qt::black);
            gradient.setColorAt(1, Qt::transparent);
            gradient.setSpread(QGradient::RepeatSpread);
            scene.setForegroundBrush(gradient);
            QRadialGradient gradient2(view.rect().center() + QPoint(int(sin(i / 1.7) * 10), int(cos(i / 1.7) * 10)), 10);
            gradient2.setColorAt(0, Qt::transparent);
            gradient2.setColorAt(0.5, Qt::black);
            gradient2.setColorAt(1, Qt::transparent);
            gradient2.setSpread(QGradient::RepeatSpread);
            scene.setBackgroundBrush(gradient2);
            QTest::qWait(10);
        }
    }

    // Test transformation extremes, see if they cause crashes
    {
        QGraphicsScene scene;
        scene.addText("GraphicsView rotated clockwise");

        QGraphicsView view(&scene);
        view.show();
        for (int i = 0; i < 160; ++i) {
            view.rotate(18);
            QTest::qWait(5);
        }
        /*
          // These cause a crash
        for (int i = 0; i < 40; ++i) {
            view.shear(1.2, 1.2);
            QTest::qWait(20);
        }
        for (int i = 0; i < 40; ++i) {
            view.shear(-1.2, -1.2);
            QTest::qWait(20);
        }
        */
        for (int i = 0; i < 20; ++i) {
            view.scale(1.2, 1.2);
            QTest::qWait(20);
        }
        for (int i = 0; i < 20; ++i) {
            view.scale(0.6, 0.6);
            QTest::qWait(20);
        }
    }
}

void tst_QGraphicsView::matrix_convenience()
{
    QGraphicsView view;
    QCOMPARE(view.matrix(), QMatrix());

    // Check the convenience functions
    view.rotate(90);
    QCOMPARE(view.matrix(), QMatrix().rotate(90));
    view.scale(2, 2);
    QCOMPARE(view.matrix(), QMatrix().scale(2, 2) * QMatrix().rotate(90));
    view.shear(1.2, 1.2);
    QCOMPARE(view.matrix(), QMatrix().shear(1.2, 1.2) * QMatrix().scale(2, 2) * QMatrix().rotate(90));
    view.translate(1, 1);
    QCOMPARE(view.matrix(), QMatrix().translate(1, 1) * QMatrix().shear(1.2, 1.2) * QMatrix().scale(2, 2) * QMatrix().rotate(90));
}

void tst_QGraphicsView::matrix_combine()
{
    // Check matrix combining
    QGraphicsView view;
    QCOMPARE(view.matrix(), QMatrix());
    view.setMatrix(QMatrix().rotate(90), true);
    view.setMatrix(QMatrix().rotate(90), true);
    view.setMatrix(QMatrix().rotate(90), true);
    view.setMatrix(QMatrix().rotate(90), true);
    QCOMPARE(view.matrix(), QMatrix());

    view.resetMatrix();
    QCOMPARE(view.matrix(), QMatrix());
    view.setMatrix(QMatrix().rotate(90), false);
    view.setMatrix(QMatrix().rotate(90), false);
    view.setMatrix(QMatrix().rotate(90), false);
    view.setMatrix(QMatrix().rotate(90), false);
    QCOMPARE(view.matrix(), QMatrix().rotate(90));
}

void tst_QGraphicsView::centerOnPoint()
{
    QGraphicsScene scene;
    scene.addEllipse(QRectF(-100, -100, 50, 50));
    scene.addEllipse(QRectF(50, -100, 50, 50));
    scene.addEllipse(QRectF(-100, 50, 50, 50));
    scene.addEllipse(QRectF(50, 50, 50, 50));

    QGraphicsView view(&scene);
    view.setSceneRect(-400, -400, 800, 800);
    view.setFixedSize(100, 100);
    view.show();

    int tolerance = 5;

    for (int i = 0; i < 3; ++i) {
        for (int y = -100; y < 100; y += 23) {
            for (int x = -100; x < 100; x += 23) {
                view.centerOn(x, y);
                QPoint viewCenter = view.mapToScene(view.viewport()->rect().center()).toPoint();

                // Fuzzy compare
                if (viewCenter.x() < x - tolerance || viewCenter.x() > x + tolerance
                    || viewCenter.y() < y - tolerance || viewCenter.y() > y + tolerance) {
                    QString error = QString("Compared values are not the same\n\tActual: (%1, %2)\n\tExpected: (%3, %4)")
                                    .arg(viewCenter.x()).arg(viewCenter.y()).arg(x).arg(y);
                    QFAIL(qPrintable(error));
                }

                QTest::qWait(1);
            }
        }

        // gneh gneh gneh
        view.rotate(13);
        view.scale(1.5, 1.5);
        view.shear(1.25, 1.25);
    }
}

void tst_QGraphicsView::centerOnItem()
{
    QGraphicsScene scene;
    QGraphicsItem *items[4];
    items[0] = scene.addEllipse(QRectF(-25, -25, 50, 50));
    items[1] = scene.addEllipse(QRectF(-25, -25, 50, 50));
    items[2] = scene.addEllipse(QRectF(-25, -25, 50, 50));
    items[3] = scene.addEllipse(QRectF(-25, -25, 50, 50));
    items[0]->setPos(-100, -100);
    items[1]->setPos(100, -100);
    items[2]->setPos(-100, 100);
    items[3]->setPos(100, 100);

    QGraphicsView view(&scene);
    view.setSceneRect(-1000, -1000, 2000, 2000);
    view.show();
    int tolerance = 7;

    for (int x = 0; x < 3; ++x) {
        for (int i = 0; i < 4; ++i) {
            view.centerOn(items[i]);

            QPoint viewCenter = view.mapToScene(view.viewport()->rect().center()).toPoint();
            qreal x = items[i]->pos().x();
            qreal y = items[i]->pos().y();

            // Fuzzy compare
            if (viewCenter.x() < x - tolerance || viewCenter.x() > x + tolerance
                || viewCenter.y() < y - tolerance || viewCenter.y() > y + tolerance) {
                QString error = QString("Compared values are not the same\n\tActual: (%1, %2)\n\tExpected: (%3, %4)")
                                .arg(viewCenter.x()).arg(viewCenter.y()).arg(x).arg(y);
                QFAIL(qPrintable(error));
            }

            QTest::qWait(250);
        }

        // gneh gneh gneh
        view.rotate(13);
        view.scale(1.5, 1.5);
        view.shear(1.25, 1.25);
    }
}

void tst_QGraphicsView::ensureVisibleRect()
{
    QGraphicsScene scene;
    QGraphicsItem *items[4];
    items[0] = scene.addEllipse(QRectF(-25, -25, 50, 50), QPen(Qt::black), QBrush(Qt::green));
    items[1] = scene.addEllipse(QRectF(-25, -25, 50, 50), QPen(Qt::black), QBrush(Qt::red));
    items[2] = scene.addEllipse(QRectF(-25, -25, 50, 50), QPen(Qt::black), QBrush(Qt::blue));
    items[3] = scene.addEllipse(QRectF(-25, -25, 50, 50), QPen(Qt::black), QBrush(Qt::yellow));
    scene.addLine(QLineF(0, -100, 0, 100), QPen(Qt::blue, 2));
    scene.addLine(QLineF(-100, 0, 100, 0), QPen(Qt::blue, 2));
    items[0]->setPos(-100, -100);
    items[1]->setPos(100, -100);
    items[2]->setPos(-100, 100);
    items[3]->setPos(100, 100);

    QGraphicsItem *icon = scene.addEllipse(QRectF(-10, -10, 20, 20), QPen(Qt::black), QBrush(Qt::gray));

    QGraphicsView view(&scene);
    view.setSceneRect(-500, -500, 1000, 1000);
    view.setFixedSize(250, 250);
    view.show();

    for (int y = -100; y < 100; y += 25) {
        for (int x = -100; x < 100; x += 13) {

            icon->setPos(x, y);

            switch (x & 3) {
            case 0:
                view.centerOn(-500, -500);
                break;
            case 1:
                view.centerOn(500, -500);
                break;
            case 2:
                view.centerOn(-500, 500);
                break;
            case 3:
            default:
                view.centerOn(500, 500);
                break;
            }

            QVERIFY(!view.viewport()->rect().contains(view.mapFromScene(x, y)));

            for (int margin = 10; margin < 60; margin += 15) {
                view.ensureVisible(x, y, 0, 0, margin, margin);

                QRect viewRect = view.viewport()->rect();
                QPoint viewPoint = view.mapFromScene(x, y);

                QVERIFY(viewRect.contains(viewPoint));
                QVERIFY(qAbs(viewPoint.x() - viewRect.left()) >= margin -1);
                QVERIFY(qAbs(viewPoint.x() - viewRect.right()) >= margin -1);
                QVERIFY(qAbs(viewPoint.y() - viewRect.top()) >= margin -1);
                QVERIFY(qAbs(viewPoint.y() - viewRect.bottom()) >= margin -1);

                QTest::qWait(10);
            }
        }
        view.rotate(5);
        view.scale(1.05, 1.05);
        view.translate(30, -30);
    }
}

void tst_QGraphicsView::fitInView()
{
    QGraphicsScene scene;
    QGraphicsItem *items[4];
    items[0] = scene.addEllipse(QRectF(-25, -25, 100, 20), QPen(Qt::black), QBrush(Qt::green));
    items[1] = scene.addEllipse(QRectF(-25, -25, 20, 100), QPen(Qt::black), QBrush(Qt::red));
    items[2] = scene.addEllipse(QRectF(-25, -25, 50, 50), QPen(Qt::black), QBrush(Qt::blue));
    items[3] = scene.addEllipse(QRectF(-25, -25, 50, 50), QPen(Qt::black), QBrush(Qt::yellow));
    scene.addLine(QLineF(0, -100, 0, 100), QPen(Qt::blue, 2));
    scene.addLine(QLineF(-100, 0, 100, 0), QPen(Qt::blue, 2));
    items[0]->setPos(-100, -100);
    items[1]->setPos(100, -100);
    items[2]->setPos(-100, 100);
    items[3]->setPos(100, 100);

    items[0]->rotate(30);
    items[1]->rotate(-30);

    QGraphicsView view(&scene);
    view.setSceneRect(-400, -400, 800, 800);
    view.setFixedSize(400, 200);
    view.show();
    view.fitInView(scene.itemsBoundingRect(), Qt::IgnoreAspectRatio);
    QTest::qWait(250);

    // Sampled coordinates.
    QVERIFY(!view.itemAt(45, 41));
    QVERIFY(!view.itemAt(297, 44));
    QVERIFY(!view.itemAt(359, 143));
    QCOMPARE(view.itemAt(79, 22), items[0]);
    QCOMPARE(view.itemAt(329, 41), items[1]);
    QCOMPARE(view.itemAt(38, 158), items[2]);
    QCOMPARE(view.itemAt(332, 160), items[3]);

    view.fitInView(items[0], Qt::IgnoreAspectRatio);
    QTest::qWait(250);
    QCOMPARE(view.itemAt(19, 13), items[0]);
    QCOMPARE(view.itemAt(91, 47), items[0]);
    QCOMPARE(view.itemAt(202, 94), items[0]);
    QCOMPARE(view.itemAt(344, 161), items[0]);
    QVERIFY(!view.itemAt(236, 54));
    QVERIFY(!view.itemAt(144, 11));
    QVERIFY(!view.itemAt(29, 69));
    QVERIFY(!view.itemAt(251, 167));

    view.fitInView(items[0], Qt::KeepAspectRatio);
    QTest::qWait(250);
    QCOMPARE(view.itemAt(328, 170), items[0]);
    QCOMPARE(view.itemAt(206, 74), items[0]);
    QCOMPARE(view.itemAt(190, 115), items[0]);
    QCOMPARE(view.itemAt(55, 14), items[0]);
    QVERIFY(!view.itemAt(109, 4));
    QVERIFY(!view.itemAt(244, 68));
    QVERIFY(!view.itemAt(310, 125));
    QVERIFY(!view.itemAt(261, 168));

    view.fitInView(items[0], Qt::KeepAspectRatioByExpanding);
    QTest::qWait(250);
    QCOMPARE(view.itemAt(18, 10), items[0]);
    QCOMPARE(view.itemAt(95, 4), items[0]);
    QCOMPARE(view.itemAt(279, 175), items[0]);
    QCOMPARE(view.itemAt(359, 170), items[0]);
    QVERIFY(!view.itemAt(370, 166));
    QVERIFY(!view.itemAt(136, 7));
    QVERIFY(!view.itemAt(31, 44));
    QVERIFY(!view.itemAt(203, 153));
}

void tst_QGraphicsView::itemsAtPoint()
{
    QGraphicsScene scene;
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(1);
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(0);
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(2);
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(-1);
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(3);

    QGraphicsView view;
    QVERIFY(view.items(0, 0).isEmpty());

    view.setScene(&scene);
    view.setSceneRect(-10000, -10000, 20000, 20000);
    view.show();

    QList<QGraphicsItem *> items = view.items(view.viewport()->rect().center());
    QCOMPARE(items.size(), 5);
    QCOMPARE(items.takeFirst()->zValue(), qreal(3));
    QCOMPARE(items.takeFirst()->zValue(), qreal(2));
    QCOMPARE(items.takeFirst()->zValue(), qreal(1));
    QCOMPARE(items.takeFirst()->zValue(), qreal(0));
    QCOMPARE(items.takeFirst()->zValue(), qreal(-1));
}

void tst_QGraphicsView::itemsInRect()
{
    QGraphicsScene scene;
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(1);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(0);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(2);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(-1);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(3);

    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(5);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(4);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(6);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(3);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(7);

    QGraphicsView view;
    QVERIFY(view.items(QRect(-100, -100, 200, 200)).isEmpty());
    view.setScene(&scene);
    view.setSceneRect(-10000, -10000, 20000, 20000);
    view.show();

    QPoint centerPoint = view.viewport()->rect().center();
    QRect leftRect(centerPoint - QPoint(40, -10), QSize(20, 20));
    QRect rightRect(centerPoint + QPoint(20, -10), QSize(20, 20));

    QList<QGraphicsItem *> items = view.items(leftRect);
    QCOMPARE(items.size(), 5);
    QCOMPARE(items.takeFirst()->zValue(), qreal(3));
    QCOMPARE(items.takeFirst()->zValue(), qreal(2));
    QCOMPARE(items.takeFirst()->zValue(), qreal(1));
    QCOMPARE(items.takeFirst()->zValue(), qreal(0));
    QCOMPARE(items.takeFirst()->zValue(), qreal(-1));

    items = view.items(rightRect);
    QCOMPARE(items.size(), 5);
    QCOMPARE(items.takeFirst()->zValue(), qreal(7));
    QCOMPARE(items.takeFirst()->zValue(), qreal(6));
    QCOMPARE(items.takeFirst()->zValue(), qreal(5));
    QCOMPARE(items.takeFirst()->zValue(), qreal(4));
    QCOMPARE(items.takeFirst()->zValue(), qreal(3));
}

void tst_QGraphicsView::itemsInPoly()
{
    QGraphicsScene scene;
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(1);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(0);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(2);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(-1);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(3);

    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(5);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(4);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(6);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(3);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(7);

    QGraphicsView view;
    QVERIFY(view.items(QPolygon()).isEmpty());
    view.setScene(&scene);
    view.setSceneRect(-10000, -10000, 20000, 20000);
    view.show();

    QPoint centerPoint = view.viewport()->rect().center();
    QPolygon leftPoly = view.mapFromScene(QRectF(-30, -10, 20, 20));
    QPolygon rightPoly = view.mapFromScene(QRectF(30, -10, 20, 20));

    QList<QGraphicsItem *> items = view.items(leftPoly);
    QCOMPARE(items.size(), 5);
    QCOMPARE(items.takeFirst()->zValue(), qreal(3));
    QCOMPARE(items.takeFirst()->zValue(), qreal(2));
    QCOMPARE(items.takeFirst()->zValue(), qreal(1));
    QCOMPARE(items.takeFirst()->zValue(), qreal(0));
    QCOMPARE(items.takeFirst()->zValue(), qreal(-1));

    items = view.items(rightPoly);
    QCOMPARE(items.size(), 5);
    QCOMPARE(items.takeFirst()->zValue(), qreal(7));
    QCOMPARE(items.takeFirst()->zValue(), qreal(6));
    QCOMPARE(items.takeFirst()->zValue(), qreal(5));
    QCOMPARE(items.takeFirst()->zValue(), qreal(4));
    QCOMPARE(items.takeFirst()->zValue(), qreal(3));
}

void tst_QGraphicsView::itemsInPath()
{
    QGraphicsScene scene;
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(1);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(0);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(2);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(-1);
    scene.addRect(QRectF(-30, -10, 20, 20))->setZValue(3);

    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(5);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(4);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(6);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(3);
    scene.addRect(QRectF(30, -10, 20, 20))->setZValue(7);

    QGraphicsView view;
    QVERIFY(view.items(QPainterPath()).isEmpty());
    view.setScene(&scene);
    view.translate(100, 400);
    view.rotate(22.3);
    view.setSceneRect(-10000, -10000, 20000, 20000);
    view.show();

    QPoint centerPoint = view.viewport()->rect().center();
    QPainterPath leftPath;
    leftPath.addEllipse(QRect(view.mapFromScene(-30, -10), QSize(20, 20)));

    QPainterPath rightPath;
    rightPath.addEllipse(QRect(view.mapFromScene(30, -10), QSize(20, 20)));

    QList<QGraphicsItem *> items = view.items(leftPath);

    QCOMPARE(items.size(), 5);
    QCOMPARE(items.takeFirst()->zValue(), qreal(3));
    QCOMPARE(items.takeFirst()->zValue(), qreal(2));
    QCOMPARE(items.takeFirst()->zValue(), qreal(1));
    QCOMPARE(items.takeFirst()->zValue(), qreal(0));
    QCOMPARE(items.takeFirst()->zValue(), qreal(-1));

    items = view.items(rightPath);
    QCOMPARE(items.size(), 5);
    QCOMPARE(items.takeFirst()->zValue(), qreal(7));
    QCOMPARE(items.takeFirst()->zValue(), qreal(6));
    QCOMPARE(items.takeFirst()->zValue(), qreal(5));
    QCOMPARE(items.takeFirst()->zValue(), qreal(4));
    QCOMPARE(items.takeFirst()->zValue(), qreal(3));
}

void tst_QGraphicsView::itemAt()
{
    QGraphicsScene scene;
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(1);
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(0);
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(2);
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(-1);
    scene.addRect(QRectF(-10, -10, 20, 20))->setZValue(3);

    QGraphicsView view;
    QCOMPARE(view.itemAt(0, 0), (QGraphicsItem *)0);

    view.setScene(&scene);
    view.setSceneRect(-10000, -10000, 20000, 20000);
    view.show();

    QCOMPARE(view.itemAt(0, 0), (QGraphicsItem *)0);
    QCOMPARE(view.itemAt(view.viewport()->rect().center())->zValue(), qreal(3));
}

void tst_QGraphicsView::mapToScene()
{
    // Uncomment the commented-out code to see what's going on. It doesn't
    // affect the test; it just slows it down.

    QGraphicsScene scene;
    scene.addPixmap(QPixmap("3D-Qt-1-2.png"));

    QGraphicsView view;
    view.setScene(&scene);
    view.setSceneRect(-500, -500, 1000, 1000);
    view.setFixedSize(300, 300);
    view.show();
    QApplication::processEvents();
    QVERIFY(view.isVisible());
    QCOMPARE(view.size(), QSize(300, 300));

    // First once without setting the scene rect
    for (int x = 0; x < view.width(); ++x) {
        for (int y = 0; y < view.height(); ++y) {
            QCOMPARE(view.mapToScene(QPoint(x, y)),
                     QPointF(view.horizontalScrollBar()->value() + x,
                             view.verticalScrollBar()->value() + y));
        }
    }

    for (int sceneRectHeight = 250; sceneRectHeight < 1000; sceneRectHeight += 250) {
        for (int sceneRectWidth = 250; sceneRectWidth < 1000; sceneRectWidth += 250) {
            view.setSceneRect(QRectF(-int(sceneRectWidth / 2), -int(sceneRectHeight / 2),
                                     sceneRectWidth, sceneRectHeight));
            QApplication::processEvents();

            int hmin = view.horizontalScrollBar()->minimum();
            int hmax = view.horizontalScrollBar()->maximum();
            int hstep = (hmax - hmin) / 3;
            int vmin = view.verticalScrollBar()->minimum();
            int vmax = view.verticalScrollBar()->maximum();
            int vstep = (vmax - vmin) / 3;

            for (int hscrollValue = hmin; hscrollValue < hmax; hscrollValue += hstep) {
                for (int vscrollValue = vmin; vscrollValue < vmax; vscrollValue += vstep) {

                    view.horizontalScrollBar()->setValue(hscrollValue);
                    view.verticalScrollBar()->setValue(vscrollValue);
                    QApplication::processEvents();

                    int h = view.horizontalScrollBar()->value();
                    int v = view.verticalScrollBar()->value();

                    for (int x = 0; x < view.width(); ++x) {
                        for (int y = 0; y < view.height(); ++y) {
                            QCOMPARE(view.mapToScene(QPoint(x, y)), QPointF(h + x, v + y));
                            QCOMPARE(view.mapFromScene(QPointF(h + x, v + y)), QPoint(x, y));
                        }
                    }
                }
            }
        }
    }
}

void tst_QGraphicsView::mapToScenePoint()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.rotate(90);
    view.setFixedSize(117, 117);
    view.show();
    QPoint center = view.viewport()->rect().center();
    QCOMPARE(view.mapToScene(center + QPoint(10, 0)),
             view.mapToScene(center) + QPointF(0, -10));
}

void tst_QGraphicsView::mapToSceneRect()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.rotate(90);
    view.setFixedSize(117, 117);
    view.show();
    QPoint center = view.viewport()->rect().center();
    QRect rect(center + QPoint(10, 0), QSize(10, 10));

    QPolygonF poly;
    poly << view.mapToScene(rect.topLeft());
    poly << view.mapToScene(rect.topRight());
    poly << view.mapToScene(rect.bottomRight());
    poly << view.mapToScene(rect.bottomLeft());

    QCOMPARE(view.mapToScene(rect), poly);
}

void tst_QGraphicsView::mapToScenePoly()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.translate(100, 100);
    view.setFixedSize(117, 117);
    view.show();
    QPoint center = view.viewport()->rect().center();
    QRect rect(center + QPoint(10, 0), QSize(10, 10));

    QPolygon poly;
    poly << rect.topLeft();
    poly << rect.topRight();
    poly << rect.bottomRight();
    poly << rect.bottomLeft();

    QPolygonF poly2;
    poly2 << view.mapToScene(rect.topLeft());
    poly2 << view.mapToScene(rect.topRight());
    poly2 << view.mapToScene(rect.bottomRight());
    poly2 << view.mapToScene(rect.bottomLeft());

    QCOMPARE(view.mapToScene(poly), poly2);
}

void tst_QGraphicsView::mapToScenePath()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.setSceneRect(-300, -300, 600, 600);
    view.translate(10, 10);
    view.setFixedSize(300, 300);
    view.show();
    QPoint center = view.viewport()->rect().center();
    QRect rect(QPoint(10, 0), QSize(10, 10));

    QPainterPath path;
    path.addRect(rect);

    QPainterPath path2;
    path2.addRect(rect.translated(view.horizontalScrollBar()->value() - 10,
                                  view.verticalScrollBar()->value() - 10));
    QCOMPARE(view.mapToScene(path), path2);
}

void tst_QGraphicsView::mapFromScenePoint()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.rotate(90);
    view.scale(10, 10);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.show();

    QPoint mapped = view.mapFromScene(0, 0);
    QPoint center = view.viewport()->rect().center();
    if (qAbs(mapped.x() - center.x()) >= 2
        || qAbs(mapped.y() - center.y()) >= 2) {
        QString error = QString("Compared values are not the same\n\tActual: (%1, %2)\n\tExpected: (%3, %4)")
                        .arg(mapped.x()).arg(mapped.y()).arg(center.x()).arg(center.y());
        QFAIL(qPrintable(error));
    }
}

void tst_QGraphicsView::mapFromSceneRect()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.rotate(90);
    view.setFixedSize(200, 200);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.show();
    QTest::qWait(25);

    QPolygon polygon;
    polygon << QPoint(98, 98);
    polygon << QPoint(98, 108);
    polygon << QPoint(88, 108);
    polygon << QPoint(88, 98);


    QPolygon viewPolygon = view.mapFromScene(0, 0, 10, 10);
    for (int i = 0; i < 4; ++i) {
        QVERIFY(qAbs(viewPolygon[i].x() - polygon[i].x()) < 3);
        QVERIFY(qAbs(viewPolygon[i].y() - polygon[i].y()) < 3);
    }
}

void tst_QGraphicsView::mapFromScenePoly()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.rotate(90);
    view.setFixedSize(200, 200);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.show();

    QPolygonF polygon;
    polygon << QPoint(0, 0);
    polygon << QPoint(10, 0);
    polygon << QPoint(10, 10);
    polygon << QPoint(0, 10);

    QPolygon polygon2;
    polygon2 << QPoint(98, 98);
    polygon2 << QPoint(98, 108);
    polygon2 << QPoint(88, 108);
    polygon2 << QPoint(88, 98);

    QPolygon viewPolygon = view.mapFromScene(polygon);
    for (int i = 0; i < 4; ++i) {
        QVERIFY(qAbs(viewPolygon[i].x() - polygon2[i].x()) < 3);
        QVERIFY(qAbs(viewPolygon[i].y() - polygon2[i].y()) < 3);
    }
}

void tst_QGraphicsView::mapFromScenePath()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.rotate(90);
    view.setFixedSize(200, 200);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.show();

    QPolygonF polygon;
    polygon << QPoint(0, 0);
    polygon << QPoint(10, 0);
    polygon << QPoint(10, 10);
    polygon << QPoint(0, 10);
    QPainterPath path;
    path.addPolygon(polygon);

    QPolygon polygon2;
    polygon2 << QPoint(98, 98);
    polygon2 << QPoint(98, 108);
    polygon2 << QPoint(88, 108);
    polygon2 << QPoint(88, 98);
    QPainterPath path2;
    path2.addPolygon(polygon2);

    QPolygonF pathPoly = view.mapFromScene(path).toFillPolygon();
    QPolygonF path2Poly = path2.toFillPolygon();

    for (int i = 0; i < pathPoly.size(); ++i) {
        QVERIFY(qAbs(pathPoly[i].x() - path2Poly[i].x()) < 3);
        QVERIFY(qAbs(pathPoly[i].y() - path2Poly[i].y()) < 3);
    }
}

void tst_QGraphicsView::sendEvent()
{
    QGraphicsScene scene;

    TestItem *item = new TestItem;
    scene.addItem(item);
    item->setFlag(QGraphicsItem::ItemIsFocusable);
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setFocus();

    QGraphicsView view(&scene);
    view.show();

    QCOMPARE(scene.focusItem(), (QGraphicsItem *)item);
    QCOMPARE(item->events.size(), 1);
    QCOMPARE(item->events.last(), QEvent::FocusIn);

    QMouseEvent mousePressEvent(QEvent::MouseButtonPress, view.rect().center(),
                                Qt::LeftButton, Qt::LeftButton, 0);
    QApplication::sendEvent(view.viewport(), &mousePressEvent);
    QCOMPARE(item->events.size(), 2);
    QCOMPARE(item->events.last(), QEvent::GraphicsSceneMousePress);

    QMouseEvent mouseMoveEvent(QEvent::MouseMove, view.rect().center(),
                                Qt::LeftButton, Qt::LeftButton, 0);
    QApplication::sendEvent(view.viewport(), &mouseMoveEvent);
    QCOMPARE(item->events.size(), 3);
    QCOMPARE(item->events.last(), QEvent::GraphicsSceneMouseMove);

    QMouseEvent mouseReleaseEvent(QEvent::MouseButtonRelease, view.rect().center(),
                                Qt::LeftButton, Qt::LeftButton, 0);
    QApplication::sendEvent(view.viewport(), &mouseReleaseEvent);
    QCOMPARE(item->events.size(), 4);
    QCOMPARE(item->events.last(), QEvent::GraphicsSceneMouseRelease);

    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Space, 0);
    QApplication::sendEvent(view.viewport(), &keyPress);
    QCOMPARE(item->events.size(), 5);
    QCOMPARE(item->events.last(), QEvent::KeyPress);
}

void tst_QGraphicsView::cursor()
{
    QGraphicsScene scene;
    QGraphicsItem *item = scene.addRect(QRectF(-10, -10, 20, 20));
    item->setCursor(Qt::IBeamCursor);

    QGraphicsView view(&scene);
    view.setFixedSize(400, 400);
    view.show();

    QCOMPARE(view.viewport()->cursor().shape(), QCursor().shape());
    view.viewport()->setCursor(Qt::PointingHandCursor);
    QCOMPARE(view.viewport()->cursor().shape(), Qt::PointingHandCursor);

    sendMouseMove(view.viewport(), view.mapFromScene(0, 0));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::IBeamCursor);

    sendMouseMove(view.viewport(), QPoint(5, 5));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::PointingHandCursor);
}

void tst_QGraphicsView::cursor2()
{
    QGraphicsScene scene;
    QGraphicsItem *item = scene.addRect(QRectF(-10, -10, 20, 20));
    item->setCursor(Qt::IBeamCursor);
    item->setZValue(1);

    QGraphicsItem *item2 = scene.addRect(QRectF(-20, -20, 40, 40));
    item2->setZValue(0);
    
    QGraphicsView view(&scene);
    view.viewport()->setCursor(Qt::PointingHandCursor);
    view.setFixedSize(400, 400);
    view.show();

    sendMouseMove(view.viewport(), view.mapFromScene(-30, -30));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::PointingHandCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(0, 0));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::IBeamCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(-30, -30));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::PointingHandCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(0, 0));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::IBeamCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(-15, 0));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::PointingHandCursor);

    view.setDragMode(QGraphicsView::ScrollHandDrag);

    sendMouseMove(view.viewport(), view.mapFromScene(-30, -30));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::OpenHandCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(0, 0));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::IBeamCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(-15, -15));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::OpenHandCursor);

    view.setDragMode(QGraphicsView::NoDrag);
    QCOMPARE(view.viewport()->cursor().shape(), Qt::ArrowCursor);
    view.viewport()->setCursor(Qt::PointingHandCursor);
    QCOMPARE(view.viewport()->cursor().shape(), Qt::PointingHandCursor);

    item2->setCursor(Qt::SizeAllCursor);

    sendMouseMove(view.viewport(), view.mapFromScene(-30, -30));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::PointingHandCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(-15, -15));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::SizeAllCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(0, 0));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::IBeamCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(-15, -15));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::SizeAllCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(0, 0));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::IBeamCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(-30, -30));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::PointingHandCursor);

    view.setDragMode(QGraphicsView::ScrollHandDrag);

    sendMouseMove(view.viewport(), view.mapFromScene(-30, -30));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::OpenHandCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(0, 0));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::IBeamCursor);
    sendMouseMove(view.viewport(), view.mapFromScene(-15, -15));
    QCOMPARE(view.viewport()->cursor().shape(), Qt::SizeAllCursor);
}

void tst_QGraphicsView::transformationAnchor()
{
    QGraphicsScene scene(-1000, -1000, 2000, 2000);
    scene.addRect(QRectF(-50, -50, 100, 100), QPen(Qt::black), QBrush(Qt::blue));

    QGraphicsView view(&scene);

    for (int i = 0; i < 2; ++i) {
        view.resize(100, 100);
        view.show();

        if (i == 0) {
            QCOMPARE(view.transformationAnchor(), QGraphicsView::AnchorViewCenter);
        } else {
            view.setTransformationAnchor(QGraphicsView::NoAnchor);
        }
        view.centerOn(0, 0);
        view.horizontalScrollBar()->setValue(100);
        QTest::qWait(100);

        QPointF center = view.mapToScene(view.viewport()->rect().center());

        view.scale(10, 10);

        QPointF newCenter = view.mapToScene(view.viewport()->rect().center());

        if (i == 0) {
            qreal slack = 3;
            QVERIFY(qAbs(newCenter.x() - center.x()) < slack);
            QVERIFY(qAbs(newCenter.y() - center.y()) < slack);
        } else {
            qreal slack = 0.3;
            QVERIFY(qAbs(newCenter.x() - center.x() / 10) < slack);
            QVERIFY(qAbs(newCenter.y() - center.y() / 10) < slack);
        }
    }
}

void tst_QGraphicsView::resizeAnchor()
{
    QGraphicsScene scene(-1000, -1000, 2000, 2000);
    scene.addRect(QRectF(-50, -50, 100, 100), QPen(Qt::black), QBrush(Qt::blue));

    QGraphicsView view(&scene);

    for (int i = 0; i < 2; ++i) {
        view.resize(100, 100);
        view.show();

        if (i == 0) {
            QCOMPARE(view.resizeAnchor(), QGraphicsView::NoAnchor);
        } else {
            view.setResizeAnchor(QGraphicsView::AnchorViewCenter);
        }
        view.centerOn(0, 0);
        QTest::qWait(100);

        QPointF f = view.mapToScene(50, 50);
        QPointF center = view.mapToScene(view.viewport()->rect().center());

        QTest::qWait(100);

        for (int size = 200; size <= 400; size += 25) {
            view.resize(size, size);
            if (i == 0) {
                QCOMPARE(view.mapToScene(50, 50), f);
                QVERIFY(view.mapToScene(view.viewport()->rect().center()) != center);
            } else {
                QVERIFY(view.mapToScene(50, 50) != f);

                QPointF newCenter = view.mapToScene(view.viewport()->rect().center());
                int slack = 3;
                QVERIFY(qAbs(newCenter.x() - center.x()) < slack);
                QVERIFY(qAbs(newCenter.y() - center.y()) < slack);
            }
            QTest::qWait(100);
        }
    }
}

QTEST_MAIN(tst_QGraphicsView)
#include "tst_qgraphicsview.moc"
#endif
