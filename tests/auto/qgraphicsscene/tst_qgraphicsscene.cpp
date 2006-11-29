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

#include <QtGui/QBitmap>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>
#include <QtGui/QScrollBar>
#include <QtGui/QStyleOption>

//TESTED_CLASS=QGraphicsScene
//TESTED_FILES=gui/graphicsview/qgraphicsscene.cpp gui/graphicsview/qgraphicsscene_bsp.cpp

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<QRectF>)
Q_DECLARE_METATYPE(QMatrix)
Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPointF)
Q_DECLARE_METATYPE(QRectF)
Q_DECLARE_METATYPE(Qt::AspectRatioMode)
Q_DECLARE_METATYPE(Qt::ItemSelectionMode)

static const int randomX[] = {276, 40, 250, 864, -56, 426, 855, 825, 184, 955, -798, -804, 773,
                              282, 489, 686, 780, -220, 50, 749, -856, -205, 81, 492, -819, 518,
                              895, 57, -559, 788, -965, 68, -442, -247, -339, -648, 292, 891,
                              -865, 462, 864, 673, 640, 523, 194, 500, -727, 307, -243, 320,
                              -545, 415, 448, 341, -619, 652, 892, -16, -14, -659, -101, -934,
                              532, 356, 824, 132, 160, 130, 104, 886, -179, -174, 543, -644, 60,
                              -470, -354, -728, 689, 682, -587, -694, -221, -741, 37, 372, -289,
                              741, -300, 858, -320, 729, -602, -956, -544, -403, 203, 398, 284,
                              -972, -572, -946, 81, 51, -403, -580, 867, 546, 565, -580, -484,
                              659, 982, -518, -976, 423, -800, 659, -297, 712, 938, -19, -16,
                              824, -252, 197, 321, -837, 824, 136, 226, -980, -909, -826, -479,
                              -835, -503, -828, -901, -810, -641, -548, -179, 194, 749, -296, 539,
                              -37, -599, -235, 121, 35, -230, -915, 789, 764, -622, -382, -90, -701,
                              676, -407, 998, 267, 913, 817, -748, -370, -162, -797, 19, -556, 933,
                              -670, -101, -765, -941, -17, 360, 31, 960, 509, 933, -35, 974, -924,
                              -734, 589, 963, 724, 794, 843, 16, -272, -811, 721, 99, -122, 216,
                              -404, 158, 787, -443, -437, -337, 383, -342, 538, -641, 791, 637,
                              -848, 397, 820, 109, 11, 45, 809, 591, 933, 961, 625, -140, -592,
                              -694, -969, 317, 293, 777, -18, -282, 835, -455, -708, -407, -204,
                              748, 347, -501, -545, 292, -362, 176, 546, -573, -38, -854, -395,
                              560, -624, -940, -971, 66, -910, 782, 985};

static const int randomY[] = {603, 70, -318, 843, 450, -637, 199, -527, 407, 964, -54, 620, -207,
                              -736, -700, -476, -706, -142, 837, 621, 522, -98, 232, 292, -267, 900,
                              615, -356, -415, 783, 290, 462, -857, -314, 677, 36, 772, 424, -72,
                              -121, 547, -533, 537, -656, 289, 508, 914, 601, 434, 588, -779, -714,
                              -368, 628, -276, 432, -1, -929, 638, -36, 253, -922, -943, 979, -34,
                              -268, -193, 601, 686, -330, 165, 98, 75, -691, -605, 617, 773, 617,
                              619, 238, -42, -405, 17, 384, -472, -846, 520, 110, 591, -217, 936,
                              -373, 731, 734, 810, 961, 881, 939, 379, -905, -137, 437, 298, 688,
                              -71, -204, 573, -120, -821, 489, -722, -926, 529, -113, -243, 543,
                              868, -301, -781, -549, -842, -489, -80, -910, -928, 51, -91, 324,
                              204, -92, 867, 723, 248, 709, -357, 591, -365, -379, 266, -649, -95,
                              205, 551, 355, -631, 79, -186, 795, -7, -225, 46, -410, 665, -874,
                              -618, 845, -548, 443, 471, -644, 606, -607, 59, -619, 288, -244, 529,
                              690, 349, -738, -611, -879, -642, 801, -178, 823, -748, -552, -247,
                              -223, -408, 651, -62, 949, -795, 171, -107, -210, -207, -842, -86,
                              436, 528, 366, -178, 245, -695, 665, 613, -948, 667, -620, -979, -949,
                              905, 181, -412, -467, -437, -774, 750, -10, 54, 205, -674, -290, -924,
                              -361, -463, 912, -702, 622, -542, 220, 115, 832, 451, -38, -952, -230,
                              -588, 864, 234, 225, -303, 493, 246, 153, 338, -378, 377, -819, 140, 136,
                              467, -849, -326, -533, 166, 252, -994, -699, 904, -566, 621, -752};

class HoverItem : public QGraphicsRectItem
{
public:
    HoverItem()
        : QGraphicsRectItem(QRectF(-10, -10, 20, 20)), isHovered(false)
    { setAcceptsHoverEvents(true); }

    bool isHovered;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
    {
        isHovered = (option->state & QStyle::State_MouseOver);

        painter->setOpacity(0.75);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::darkGray);
        painter->drawRoundRect(boundingRect().adjusted(3, 3, -3, -3), Qt::darkGray);
        painter->setPen(Qt::black);
        if (isHovered) {
            painter->setBrush(QColor(Qt::blue).light(120));
        } else {
            painter->setBrush(Qt::gray);
        }
        painter->drawRoundRect(boundingRect().adjusted(0, 0, -5, -5));
    }
};

class tst_QGraphicsScene : public QObject
{
    Q_OBJECT

private slots:
    void construction();
    void sceneRect();
    void itemIndexMethod();
    void itemsBoundingRect_data();
    void itemsBoundingRect();
    void items();
    void items_QPointF_data();
    void items_QPointF();
    void items_QRectF();
    void items_QRectF_2_data();
    void items_QRectF_2();
    void items_QPolygonF();
    void items_QPolygonF_2();
    void items_QPainterPath();
    void items_QPainterPath_2();
    void selection();
    void addItem();
    void addEllipse();
    void addLine();
    void addPath();
    void addPixmap();
    void addRect();
    void addText();
    void removeItem();
    void focusItem();
    void setFocusItem();
    void mouseGrabberItem();
    void hoverEvents_siblings();
    void hoverEvents_parentChild();
    void createItemGroup();
    void mouseEventPropagation();
    void mouseEventPropagation_ignore();
    void mouseEventPropagation_focus();
    void mouseEventPropagation_doubleclick();
    void mouseEventPropagation_mouseMove();
    void dragAndDrop_simple();
    void dragAndDrop_disabledOrInvisible();
    void dragAndDrop_propagate();
    void render_data();
    void render();
    void contextMenuEvent();
    void update();
    void views();
    void event();

    // task specific tests below me
    void task139710_bspTreeCrash();
    void task139782_containsItemBoundingRect();
};

void tst_QGraphicsScene::construction()
{
    QGraphicsScene scene;
    QCOMPARE(scene.itemsBoundingRect(), QRectF());
    QVERIFY(scene.items().isEmpty());
    QVERIFY(scene.items(QPointF()).isEmpty());
    QVERIFY(scene.items(QRectF()).isEmpty());
    QVERIFY(scene.items(QPolygonF()).isEmpty());
    QVERIFY(scene.items(QPainterPath()).isEmpty());
    QTest::ignoreMessage(QtWarningMsg, "QGraphicsScene::collidingItems: cannot find collisions for null item");
    QVERIFY(scene.collidingItems(0).isEmpty());
    QVERIFY(!scene.itemAt(QPointF()));
    QVERIFY(scene.selectedItems().isEmpty());
    QVERIFY(!scene.focusItem());
}

void tst_QGraphicsScene::sceneRect()
{
    QGraphicsScene scene;
    QCOMPARE(scene.sceneRect(), QRectF());

    QGraphicsItem *item = scene.addRect(QRectF(0, 0, 10, 10));
    item->setPos(-5, -5);

    QCOMPARE(scene.itemAt(0, 0), item);
    QCOMPARE(scene.itemAt(10, 10), (QGraphicsItem *)0);
    QCOMPARE(scene.sceneRect(), QRectF(-5, -5, 15, 15));

    scene.setSceneRect(-100, -100, 10, 10);

    QCOMPARE(scene.itemAt(0, 0), item);
    QCOMPARE(scene.itemAt(10, 10), (QGraphicsItem *)0);
    QCOMPARE(scene.sceneRect(), QRectF(-100, -100, 10, 10));

    scene.setSceneRect(QRectF());

    QCOMPARE(scene.itemAt(0, 0), item);
    QCOMPARE(scene.itemAt(10, 10), (QGraphicsItem *)0);
    QCOMPARE(scene.sceneRect(), QRectF(-5, -5, 15, 15));
}

void tst_QGraphicsScene::itemIndexMethod()
{
    QGraphicsScene scene;
    QCOMPARE(scene.itemIndexMethod(), QGraphicsScene::BspTreeIndex);

    QList<QGraphicsItem *> items;
    for (int y = -1000; y < 2000; y += 100) {
        for (int x = -1000; x < 2000; x += 100) {
            QGraphicsItem *item = scene.addRect(QRectF(0, 0, 10, 10));
            item->setPos(x, y);
            QCOMPARE(scene.itemAt(x, y), item);
            items << item;
        }
    }

    int n = 0;
    for (int y = -1000; y < 2000; y += 100) {
        for (int x = -1000; x < 2000; x += 100)
            QCOMPARE(scene.itemAt(x, y), items.at(n++));
    }

    scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    QCOMPARE(scene.itemIndexMethod(), QGraphicsScene::NoIndex);

    n = 0;
    for (int y = -1000; y < 2000; y += 100) {
        for (int x = -1000; x < 2000; x += 100)
            QCOMPARE(scene.itemAt(x, y), items.at(n++));
    }

    scene.setItemIndexMethod(QGraphicsScene::BspTreeIndex);
    QCOMPARE(scene.itemIndexMethod(), QGraphicsScene::BspTreeIndex);

    n = 0;
    for (int y = -1000; y < 2000; y += 100) {
        for (int x = -1000; x < 2000; x += 100)
            QCOMPARE(scene.itemAt(x, y), items.at(n++));
    }
}

void tst_QGraphicsScene::items()
{
    QGraphicsScene scene;

    QList<QGraphicsItem *> items;
    for (int y = -1000; y < 2000; y += 100) {
        for (int x = -1000; x < 2000; x += 100)
            items << scene.addRect(QRectF(0, 0, 10, 10));
    }

    QCOMPARE(scene.items(), items);
    scene.itemAt(0, 0); // trigger indexing

    scene.removeItem(items.at(5));
    delete items.at(5);
    QVERIFY(!scene.items().contains(0));
    delete items.at(7);
    QVERIFY(!scene.items().contains(0));
}

void tst_QGraphicsScene::itemsBoundingRect_data()
{
    QTest::addColumn<QList<QRectF> >("rects");
    QTest::addColumn<QMatrix>("matrix");
    QTest::addColumn<QRectF>("boundingRect");

    QMatrix transformationMatrix;
    transformationMatrix.translate(50, -50);
    transformationMatrix.scale(2, 2);
    transformationMatrix.rotate(90);

    QTest::newRow("none")
        << QList<QRectF>()
        << QMatrix()
        << QRectF();
    QTest::newRow("{{0, 0, 10, 10}}")
        << (QList<QRectF>() << QRectF(0, 0, 10, 10))
        << QMatrix()
        << QRectF(0, 0, 10, 10);
    QTest::newRow("{{-10, -10, 10, 10}}")
        << (QList<QRectF>() << QRectF(-10, -10, 10, 10))
        << QMatrix()
        << QRectF(-10, -10, 10, 10);
    QTest::newRow("{{-1000, -1000, 1, 1}, {-10, -10, 10, 10}}")
        << (QList<QRectF>() << QRectF(-1000, -1000, 1, 1) << QRectF(-10, -10, 10, 10))
        << QMatrix()
        << QRectF(-1000, -1000, 1000, 1000);
    QTest::newRow("transformed {{0, 0, 10, 10}}")
        << (QList<QRectF>() << QRectF(0, 0, 10, 10))
        << transformationMatrix
        << QRectF(30, -50, 20, 20);
    QTest::newRow("transformed {{-10, -10, 10, 10}}")
        << (QList<QRectF>() << QRectF(-10, -10, 10, 10))
        << transformationMatrix
        << QRectF(50, -70, 20, 20);
    QTest::newRow("transformed {{-1000, -1000, 1, 1}, {-10, -10, 10, 10}}")
        << (QList<QRectF>() << QRectF(-1000, -1000, 1, 1) << QRectF(-10, -10, 10, 10))
        << transformationMatrix
        << QRectF(50, -2050, 2000, 2000);

    QList<QRectF> all;
    for (int i = 0; i < 256; ++i)
        all << QRectF(randomX[i], randomY[i], 10, 10);
    QTest::newRow("all")
        << all
        << QMatrix()
        << QRectF(-980, -994, 1988, 1983);
    QTest::newRow("transformed all")
        << all
        << transformationMatrix
        << QRectF(-1928, -2010, 3966, 3976);
}

void tst_QGraphicsScene::itemsBoundingRect()
{
    QFETCH(QList<QRectF>, rects);
    QFETCH(QMatrix, matrix);
    QFETCH(QRectF, boundingRect);

    QGraphicsScene scene;

    foreach (QRectF rect, rects) {
        QPainterPath path;
        path.addRect(rect);
        scene.addPath(path)->setMatrix(matrix);
    }

    QCOMPARE(scene.itemsBoundingRect(), boundingRect);
}

void tst_QGraphicsScene::items_QPointF_data()
{
    QTest::addColumn<QList<QRectF> >("items");
    QTest::addColumn<QPointF>("point");
    QTest::addColumn<QList<int> >("itemsAtPoint");

    QTest::newRow("empty")
        << QList<QRectF>()
        << QPointF()
        << QList<int>();
    QTest::newRow("1")
        << (QList<QRectF>() << QRectF(0, 0, 10, 10))
        << QPointF(0, 0)
        << (QList<int>() << 0);
    QTest::newRow("2")
        << (QList<QRectF>() << QRectF(0, 0, 10, 10))
        << QPointF(5, 5)
        << (QList<int>() << 0);
    QTest::newRow("3")
        << (QList<QRectF>() << QRectF(0, 0, 10, 10))
        << QPointF(10, 10)
        << (QList<int>() << 0);
    QTest::newRow("4")
        << (QList<QRectF>() << QRectF(0, 0, 10, 10) << QRectF(10, 10, 10, 10))
        << QPointF(10, 10)
        << (QList<int>() << 1 << 0);
    QTest::newRow("5")
        << (QList<QRectF>() << QRectF(5, 5, 10, 10) << QRectF(10, 10, 10, 10))
        << QPointF(10, 10)
        << (QList<int>() << 1 << 0);
    QTest::newRow("6")
        << (QList<QRectF>() << QRectF(5, 5, 10, 10) << QRectF(10, 10, 10, 10) << QRectF(0, 0, 20, 30))
        << QPointF(10, 10)
        << (QList<int>() << 2 << 1 << 0);
}

void tst_QGraphicsScene::items_QPointF()
{
    QFETCH(QList<QRectF>, items);
    QFETCH(QPointF, point);
    QFETCH(QList<int>, itemsAtPoint);

    QGraphicsScene scene;

    int n = 0;
    QList<QGraphicsItem *> addedItems;
    foreach(QRectF rect, items) {
        QPainterPath path;
        path.addRect(0, 0, rect.width(), rect.height());

        QGraphicsItem *item = scene.addPath(path);
        item->setZValue(n++);
        item->setPos(rect.topLeft());
        addedItems << item;
    }

    QList<int> itemIndexes;
    foreach (QGraphicsItem *item, scene.items(point))
        itemIndexes << addedItems.indexOf(item);

    QCOMPARE(itemIndexes, itemsAtPoint);
}

void tst_QGraphicsScene::items_QRectF()
{
    QGraphicsScene scene;
    QGraphicsItem *item1 = scene.addRect(QRectF(-10, -10, 10, 10));
    QGraphicsItem *item2 = scene.addRect(QRectF(10, -10, 10, 10));
    QGraphicsItem *item3 = scene.addRect(QRectF(10, 10, 10, 10));
    QGraphicsItem *item4 = scene.addRect(QRectF(-10, 10, 10, 10));

    item1->setZValue(0);
    item2->setZValue(1);
    item3->setZValue(2);
    item4->setZValue(3);

    QCOMPARE(scene.items(QRectF(-10, -10, 10, 10)), QList<QGraphicsItem *>() << item1);
    QCOMPARE(scene.items(QRectF(10, -10, 10, 10)), QList<QGraphicsItem *>() << item2);
    QCOMPARE(scene.items(QRectF(10, 10, 10, 10)), QList<QGraphicsItem *>() << item3);
    QCOMPARE(scene.items(QRectF(-10, 10, 10, 10)), QList<QGraphicsItem *>() << item4);
    QCOMPARE(scene.items(QRectF(-10, -10, 1, 1)), QList<QGraphicsItem *>() << item1);
    QCOMPARE(scene.items(QRectF(10, -10, 1, 1)), QList<QGraphicsItem *>() << item2);
    QCOMPARE(scene.items(QRectF(10, 10, 1, 1)), QList<QGraphicsItem *>() << item3);
    QCOMPARE(scene.items(QRectF(-10, 10, 1, 1)), QList<QGraphicsItem *>() << item4);

    QCOMPARE(scene.items(QRectF(-10, -10, 40, 10)), QList<QGraphicsItem *>() << item2 << item1);
    QCOMPARE(scene.items(QRectF(-10, 10, 40, 10)), QList<QGraphicsItem *>() << item4 << item3);

    item1->setZValue(3);
    item2->setZValue(2);
    item3->setZValue(1);
    item4->setZValue(0);

    QCOMPARE(scene.items(QRectF(-10, -10, 40, 10)), QList<QGraphicsItem *>() << item1 << item2);
    QCOMPARE(scene.items(QRectF(-10, 10, 40, 10)), QList<QGraphicsItem *>() << item3 << item4);
}

void tst_QGraphicsScene::items_QRectF_2_data()
{
    QTest::addColumn<QRectF>("ellipseRect");
    QTest::addColumn<QRectF>("sceneRect");
    QTest::addColumn<Qt::ItemSelectionMode>("selectionMode");
    QTest::addColumn<bool>("contained");

    // None of the rects contain the ellipse's shape nor bounding rect
    QTest::newRow("1") << QRectF(0, 0, 100, 100) << QRectF(1, 1, 10, 10) << Qt::ContainsItemShape << false;
    QTest::newRow("2") << QRectF(0, 0, 100, 100) << QRectF(1, 89, 10, 10) << Qt::ContainsItemShape << false;
    QTest::newRow("3") << QRectF(0, 0, 100, 100) << QRectF(89, 1, 10, 10) << Qt::ContainsItemShape << false;
    QTest::newRow("4") << QRectF(0, 0, 100, 100) << QRectF(89, 89, 10, 10) << Qt::ContainsItemShape << false;
    QTest::newRow("5") << QRectF(0, 0, 100, 100) << QRectF(1, 1, 10, 10) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("6") << QRectF(0, 0, 100, 100) << QRectF(1, 89, 10, 10) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("7") << QRectF(0, 0, 100, 100) << QRectF(89, 1, 10, 10) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("8") << QRectF(0, 0, 100, 100) << QRectF(89, 89, 10, 10) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("9") << QRectF(0, 0, 100, 100) << QRectF(0, 0, 50, 50) << Qt::ContainsItemShape << false;
    QTest::newRow("10") << QRectF(0, 0, 100, 100) << QRectF(0, 50, 50, 50) << Qt::ContainsItemShape << false;
    QTest::newRow("11") << QRectF(0, 0, 100, 100) << QRectF(50, 0, 50, 50) << Qt::ContainsItemShape << false;
    QTest::newRow("12") << QRectF(0, 0, 100, 100) << QRectF(50, 50, 50, 50) << Qt::ContainsItemShape << false;
    QTest::newRow("13") << QRectF(0, 0, 100, 100) << QRectF(0, 0, 50, 50) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("14") << QRectF(0, 0, 100, 100) << QRectF(0, 50, 50, 50) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("15") << QRectF(0, 0, 100, 100) << QRectF(50, 0, 50, 50) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("16") << QRectF(0, 0, 100, 100) << QRectF(50, 50, 50, 50) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("17") << QRectF(0, 0, 100, 100) << QRectF(-50, -50, 100, 100) << Qt::ContainsItemShape << false;
    QTest::newRow("18") << QRectF(0, 0, 100, 100) << QRectF(0, -50, 100, 100) << Qt::ContainsItemShape << false;
    QTest::newRow("19") << QRectF(0, 0, 100, 100) << QRectF(-50, 0, 100, 100) << Qt::ContainsItemShape << false;
    QTest::newRow("20") << QRectF(0, 0, 100, 100) << QRectF(0, 0, 100, 100) << Qt::ContainsItemShape << false;
    QTest::newRow("21") << QRectF(0, 0, 100, 100) << QRectF(-50, -50, 100, 100) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("22") << QRectF(0, 0, 100, 100) << QRectF(0, -50, 100, 100) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("23") << QRectF(0, 0, 100, 100) << QRectF(-50, 0, 100, 100) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("24") << QRectF(0, 0, 100, 100) << QRectF(0, 0, 100, 100) << Qt::ContainsItemBoundingRect << false;

    // None intersects with the item's shape, but they all intersects with the
    // item's bounding rect.
    QTest::newRow("25") << QRectF(0, 0, 100, 100) << QRectF(1, 1, 10, 10) << Qt::IntersectsItemShape << false;
    QTest::newRow("26") << QRectF(0, 0, 100, 100) << QRectF(1, 89, 10, 10) << Qt::IntersectsItemShape << false;
    QTest::newRow("27") << QRectF(0, 0, 100, 100) << QRectF(89, 1, 10, 10) << Qt::IntersectsItemShape << false;
    QTest::newRow("28") << QRectF(0, 0, 100, 100) << QRectF(89, 89, 10, 10) << Qt::IntersectsItemShape << false;
    QTest::newRow("29") << QRectF(0, 0, 100, 100) << QRectF(1, 1, 10, 10) << Qt::IntersectsItemBoundingRect << true;
    QTest::newRow("30") << QRectF(0, 0, 100, 100) << QRectF(1, 89, 10, 10) << Qt::IntersectsItemBoundingRect << true;
    QTest::newRow("31") << QRectF(0, 0, 100, 100) << QRectF(89, 1, 10, 10) << Qt::IntersectsItemBoundingRect << true;
    QTest::newRow("32") << QRectF(0, 0, 100, 100) << QRectF(89, 89, 10, 10) << Qt::IntersectsItemBoundingRect << true;

    // This rect does not contain the shape nor the bounding rect
    QTest::newRow("33") << QRectF(0, 0, 100, 100) << QRectF(5, 5, 90, 90) << Qt::ContainsItemShape << false;
    QTest::newRow("34") << QRectF(0, 0, 100, 100) << QRectF(5, 5, 90, 90) << Qt::ContainsItemBoundingRect << false;

    // It will, however, intersect with both
    QTest::newRow("35") << QRectF(0, 0, 100, 100) << QRectF(5, 5, 90, 90) << Qt::IntersectsItemShape << true;
    QTest::newRow("36") << QRectF(0, 0, 100, 100) << QRectF(5, 5, 90, 90) << Qt::IntersectsItemBoundingRect << true;

    // A rect that contains the whole ellipse will both contain and intersect
    // with both the ellipse's shape and bounding rect.
    QTest::newRow("37") << QRectF(0, 0, 100, 100) << QRectF(-5, -5, 110, 110) << Qt::IntersectsItemBoundingRect << true;
    QTest::newRow("38") << QRectF(0, 0, 100, 100) << QRectF(-5, -5, 110, 110) << Qt::IntersectsItemShape << true;
    QTest::newRow("39") << QRectF(0, 0, 100, 100) << QRectF(-5, -5, 110, 110) << Qt::ContainsItemBoundingRect << true;
    QTest::newRow("40") << QRectF(0, 0, 100, 100) << QRectF(-5, -5, 110, 110) << Qt::ContainsItemShape << true;

    // A rect that is fully contained within the ellipse will intersect only
    QTest::newRow("41") << QRectF(0, 0, 100, 100) << QRectF(40, 40, 20, 20) << Qt::ContainsItemShape << false;
    QTest::newRow("42") << QRectF(0, 0, 100, 100) << QRectF(40, 40, 20, 20) << Qt::ContainsItemBoundingRect << false;
    QTest::newRow("43") << QRectF(0, 0, 100, 100) << QRectF(40, 40, 20, 20) << Qt::IntersectsItemShape << true;
    QTest::newRow("44") << QRectF(0, 0, 100, 100) << QRectF(40, 40, 20, 20) << Qt::IntersectsItemBoundingRect << true;
}

void tst_QGraphicsScene::items_QRectF_2()
{
    QFETCH(QRectF, ellipseRect);
    QFETCH(QRectF, sceneRect);
    QFETCH(Qt::ItemSelectionMode, selectionMode);
    QFETCH(bool, contained);

    QGraphicsScene scene;
    QGraphicsItem *item = scene.addEllipse(ellipseRect);

    QEXPECT_FAIL("21", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("22", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("23", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QCOMPARE(!scene.items(sceneRect, selectionMode).isEmpty(), contained);
    item->rotate(45);
    QEXPECT_FAIL("13", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("21", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("22", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("23", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("24", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("26", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("31", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("32", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QEXPECT_FAIL("40", "Fix this when we get painterpath intersection determination, task 139782", Continue);
    QCOMPARE(!scene.items(sceneRect, selectionMode).isEmpty(), contained);
}

void tst_QGraphicsScene::items_QPolygonF()
{
    QGraphicsScene scene;
    QGraphicsItem *item1 = scene.addRect(QRectF(-10, -10, 10, 10));
    QGraphicsItem *item2 = scene.addRect(QRectF(10, -10, 10, 10));
    QGraphicsItem *item3 = scene.addRect(QRectF(10, 10, 10, 10));
    QGraphicsItem *item4 = scene.addRect(QRectF(-10, 10, 10, 10));

    item1->setZValue(0);
    item2->setZValue(1);
    item3->setZValue(2);
    item4->setZValue(3);

    QPolygonF poly1(item1->boundingRect());
    QPolygonF poly2(item2->boundingRect());
    QPolygonF poly3(item3->boundingRect());
    QPolygonF poly4(item4->boundingRect());

    QCOMPARE(scene.items(poly1), QList<QGraphicsItem *>() << item1);
    QCOMPARE(scene.items(poly2), QList<QGraphicsItem *>() << item2);
    QCOMPARE(scene.items(poly3), QList<QGraphicsItem *>() << item3);
    QCOMPARE(scene.items(poly4), QList<QGraphicsItem *>() << item4);

    poly1 = QPolygonF(QRectF(-10, -10, 1, 1));
    poly2 = QPolygonF(QRectF(10, -10, 1, 1));
    poly3 = QPolygonF(QRectF(10, 10, 1, 1));
    poly4 = QPolygonF(QRectF(-10, 10, 1, 1));

    QCOMPARE(scene.items(poly1), QList<QGraphicsItem *>() << item1);
    QCOMPARE(scene.items(poly2), QList<QGraphicsItem *>() << item2);
    QCOMPARE(scene.items(poly3), QList<QGraphicsItem *>() << item3);
    QCOMPARE(scene.items(poly4), QList<QGraphicsItem *>() << item4);

    poly1 = QPolygonF(QRectF(-10, -10, 40, 10));
    poly2 = QPolygonF(QRectF(-10, 10, 40, 10));

    QCOMPARE(scene.items(poly1), QList<QGraphicsItem *>() << item2 << item1);
    QCOMPARE(scene.items(poly2), QList<QGraphicsItem *>() << item4 << item3);

    item1->setZValue(3);
    item2->setZValue(2);
    item3->setZValue(1);
    item4->setZValue(0);

    QCOMPARE(scene.items(poly1), QList<QGraphicsItem *>() << item1 << item2);
    QCOMPARE(scene.items(poly2), QList<QGraphicsItem *>() << item3 << item4);
}

void tst_QGraphicsScene::items_QPolygonF_2()
{
    QGraphicsScene scene;
    QGraphicsItem *ellipse = scene.addEllipse(QRectF(0, 0, 100, 100));
    
    // None of the rects contain the ellipse's shape nor bounding rect
    QVERIFY(scene.items(QPolygonF(QRectF(1, 1, 10, 10)), Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(1, 89, 10, 10)), Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(89, 1, 10, 10)), Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(89, 89, 10, 10)), Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(1, 1, 10, 10)), Qt::ContainsItemBoundingRect).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(1, 89, 10, 10)), Qt::ContainsItemBoundingRect).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(89, 1, 10, 10)), Qt::ContainsItemBoundingRect).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(89, 89, 10, 10)), Qt::ContainsItemBoundingRect).isEmpty());

    // None intersects with the item's shape, but they all intersects with the
    // item's bounding rect.
    QVERIFY(scene.items(QPolygonF(QRectF(1, 1, 10, 10)), Qt::IntersectsItemShape).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(1, 89, 10, 10)), Qt::IntersectsItemShape).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(89, 1, 10, 10)), Qt::IntersectsItemShape).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(89, 89, 10, 10)), Qt::IntersectsItemShape).isEmpty());
    QCOMPARE(scene.items(QPolygonF(QRectF(1, 1, 10, 10)), Qt::IntersectsItemBoundingRect).first(), ellipse);
    QCOMPARE(scene.items(QPolygonF(QRectF(1, 89, 10, 10)), Qt::IntersectsItemBoundingRect).first(), ellipse);
    QCOMPARE(scene.items(QPolygonF(QRectF(89, 1, 10, 10)), Qt::IntersectsItemBoundingRect).first(), ellipse);
    QCOMPARE(scene.items(QPolygonF(QRectF(89, 89, 10, 10)), Qt::IntersectsItemBoundingRect).first(), ellipse);

    // This rect does not contain the shape nor the bounding rect
    QVERIFY(scene.items(QPolygonF(QRectF(5, 5, 90, 90)), Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(QPolygonF(QRectF(5, 5, 90, 90)), Qt::ContainsItemBoundingRect).isEmpty());

    // It will, however, intersect with both
    QCOMPARE(scene.items(QPolygonF(QRectF(5, 5, 90, 90)), Qt::IntersectsItemShape).first(), ellipse);
    QCOMPARE(scene.items(QPolygonF(QRectF(5, 5, 90, 90)), Qt::IntersectsItemBoundingRect).first(), ellipse);

    // A rect that contains the whole ellipse will both contain and intersect
    // with both the ellipse's shape and bounding rect.
    QCOMPARE(scene.items(QPolygonF(QRectF(-5, -5, 110, 110)), Qt::IntersectsItemShape).first(), ellipse);
    QCOMPARE(scene.items(QPolygonF(QRectF(-5, -5, 110, 110)), Qt::ContainsItemShape).first(), ellipse);
    QCOMPARE(scene.items(QPolygonF(QRectF(-5, -5, 110, 110)), Qt::IntersectsItemBoundingRect).first(), ellipse);
    QCOMPARE(scene.items(QPolygonF(QRectF(-5, -5, 110, 110)), Qt::ContainsItemBoundingRect).first(), ellipse);
}

void tst_QGraphicsScene::items_QPainterPath()
{
    QGraphicsScene scene;
    QGraphicsItem *item1 = scene.addRect(QRectF(-10, -10, 10, 10));
    QGraphicsItem *item2 = scene.addRect(QRectF(10, -10, 10, 10));
    QGraphicsItem *item3 = scene.addRect(QRectF(10, 10, 10, 10));
    QGraphicsItem *item4 = scene.addRect(QRectF(-10, 10, 10, 10));

    item1->setZValue(0);
    item2->setZValue(1);
    item3->setZValue(2);
    item4->setZValue(3);

    QPainterPath path1; path1.addEllipse(item1->boundingRect());
    QPainterPath path2; path2.addEllipse(item2->boundingRect());
    QPainterPath path3; path3.addEllipse(item3->boundingRect());
    QPainterPath path4; path4.addEllipse(item4->boundingRect());

    QCOMPARE(scene.items(path1), QList<QGraphicsItem *>() << item1);
    QCOMPARE(scene.items(path2), QList<QGraphicsItem *>() << item2);
    QCOMPARE(scene.items(path3), QList<QGraphicsItem *>() << item3);
    QCOMPARE(scene.items(path4), QList<QGraphicsItem *>() << item4);

    path1 = QPainterPath(); path1.addEllipse(QRectF(-10, -10, 1, 1));
    path2 = QPainterPath(); path2.addEllipse(QRectF(10, -10, 1, 1));
    path3 = QPainterPath(); path3.addEllipse(QRectF(10, 10, 1, 1));
    path4 = QPainterPath(); path4.addEllipse(QRectF(-10, 10, 1, 1));

    QCOMPARE(scene.items(path1), QList<QGraphicsItem *>() << item1);
    QCOMPARE(scene.items(path2), QList<QGraphicsItem *>() << item2);
    QCOMPARE(scene.items(path3), QList<QGraphicsItem *>() << item3);
    QCOMPARE(scene.items(path4), QList<QGraphicsItem *>() << item4);

    path1 = QPainterPath(); path1.addRect(QRectF(-10, -10, 40, 10));
    path2 = QPainterPath(); path2.addRect(QRectF(-10, 10, 40, 10));

    QCOMPARE(scene.items(path1), QList<QGraphicsItem *>() << item2 << item1);
    QCOMPARE(scene.items(path2), QList<QGraphicsItem *>() << item4 << item3);

    item1->setZValue(3);
    item2->setZValue(2);
    item3->setZValue(1);
    item4->setZValue(0);

    QCOMPARE(scene.items(path1), QList<QGraphicsItem *>() << item1 << item2);
    QCOMPARE(scene.items(path2), QList<QGraphicsItem *>() << item3 << item4);
}

void tst_QGraphicsScene::items_QPainterPath_2()
{
    QGraphicsScene scene;
    QGraphicsItem *ellipse = scene.addEllipse(QRectF(0, 0, 100, 100));

    QPainterPath p1; p1.addRect(QRectF(1, 1, 10, 10));
    QPainterPath p2; p2.addRect(QRectF(1, 89, 10, 10));
    QPainterPath p3; p3.addRect(QRectF(89, 1, 10, 10));
    QPainterPath p4; p4.addRect(QRectF(89, 89, 10, 10));
    
    // None of the rects contain the ellipse's shape nor bounding rect
    QVERIFY(scene.items(p1, Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(p2, Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(p3, Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(p4, Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(p1, Qt::ContainsItemBoundingRect).isEmpty());
    QVERIFY(scene.items(p2, Qt::ContainsItemBoundingRect).isEmpty());
    QVERIFY(scene.items(p3, Qt::ContainsItemBoundingRect).isEmpty());
    QVERIFY(scene.items(p4, Qt::ContainsItemBoundingRect).isEmpty());

    // None intersects with the item's shape, but they all intersects with the
    // item's bounding rect.
    QVERIFY(scene.items(p1, Qt::IntersectsItemShape).isEmpty());
    QVERIFY(scene.items(p2, Qt::IntersectsItemShape).isEmpty());
    QVERIFY(scene.items(p3, Qt::IntersectsItemShape).isEmpty());
    QVERIFY(scene.items(p4, Qt::IntersectsItemShape).isEmpty());
    QCOMPARE(scene.items(p1, Qt::IntersectsItemBoundingRect).first(), ellipse);
    QCOMPARE(scene.items(p2, Qt::IntersectsItemBoundingRect).first(), ellipse);
    QCOMPARE(scene.items(p3, Qt::IntersectsItemBoundingRect).first(), ellipse);
    QCOMPARE(scene.items(p4, Qt::IntersectsItemBoundingRect).first(), ellipse);

    QPainterPath p5;
    p5.addRect(QRectF(5, 5, 90, 90));
    
    // This rect does not contain the shape nor the bounding rect
    QVERIFY(scene.items(p5, Qt::ContainsItemShape).isEmpty());
    QVERIFY(scene.items(p5, Qt::ContainsItemBoundingRect).isEmpty());

    // It will, however, intersect with both
    QCOMPARE(scene.items(p5, Qt::IntersectsItemShape).first(), ellipse);
    QCOMPARE(scene.items(p5, Qt::IntersectsItemBoundingRect).first(), ellipse);

    QPainterPath p6;
    p6.addRect(QRectF(-5, -5, 110, 110));

    // A rect that contains the whole ellipse will both contain and intersect
    // with both the ellipse's shape and bounding rect.
    QCOMPARE(scene.items(p6, Qt::IntersectsItemShape).first(), ellipse);
    QCOMPARE(scene.items(p6, Qt::ContainsItemShape).first(), ellipse);
    QCOMPARE(scene.items(p6, Qt::IntersectsItemBoundingRect).first(), ellipse);
    QCOMPARE(scene.items(p6, Qt::ContainsItemBoundingRect).first(), ellipse);
}

void tst_QGraphicsScene::selection()
{
    QGraphicsScene scene;
    QMap<QGraphicsItem *, int> itemIndexes;
    for (int i = 0; i < 256; ++i) {
        QPainterPath path;
        path.addRect(randomX[i], randomY[i], 25, 25);

        QGraphicsPathItem *pathItem = scene.addPath(path);
        pathItem->setFlag(QGraphicsItem::ItemIsSelectable);
        itemIndexes.insert(pathItem, i);
    }

#if 0
    // Write data
    QFile::remove("graphicsScene_selection.data");
    QFile file("graphicsScene_selection.data");
    if (!file.open(QFile::WriteOnly))
        QFAIL("Unable to generate data file graphicsScene_selection.data");
    QDataStream stream(&file);
    for (qreal y = -1000; y < 1000; y += 33) {
        for (qreal x = -1000; x < 1000; x += 33) {
            for (qreal size = 1; size < 200; size += 33) {
                QPainterPath path;
                path.addRect(QRectF(x, y, size, size));
                scene.setSelectionArea(path);

                QList<int> indexes;
                foreach (QGraphicsItem *item, scene.selectedItems())
                    indexes << itemIndexes.value(item);

                stream << x << y << size << indexes;
            }
        }
    }
#else
    // Read data
    QFile file("graphicsScene_selection.data");
    if (!file.open(QFile::ReadOnly))
        QFAIL("Unable to load data file graphicsScene_selection.data");

    QDataStream stream(&file);

    while (!stream.atEnd()) {
        QList<int> expectedIndexes;

        qreal x, y, size;
        stream >> x >> y >> size >> expectedIndexes;

        QPainterPath path;
        path.addRect(QRectF(x, y, size, size));
        scene.setSelectionArea(path);

        QList<int> indexes;
        foreach (QGraphicsItem *item, scene.selectedItems())
            indexes << itemIndexes.value(item);

        qSort(indexes);
        qSort(expectedIndexes);

        QCOMPARE(indexes, expectedIndexes);

        scene.clearSelection();
        QVERIFY(scene.selectedItems().isEmpty());
    }
#endif
}

void tst_QGraphicsScene::addItem()
{
    {
        // 1) Create item, then scene, then add item
        QGraphicsItem *path = new QGraphicsEllipseItem(QRectF(-10, -10, 20, 20));
        QGraphicsScene scene;
        scene.addItem(path);

        QCOMPARE(scene.itemAt(0, 0), path);

        QGraphicsItem *path2 = new QGraphicsEllipseItem(QRectF(-10, -10, 20, 20));
        path2->setPos(100, 100);

        QCOMPARE(scene.itemAt(0, 0), path);
        QCOMPARE(scene.itemAt(100, 100), (QGraphicsItem *)0);
        scene.addItem(path2);
        QCOMPARE(scene.itemAt(100, 100), path2);
    }
    {
        // 2) Create scene, then item, then add item
        QGraphicsScene scene;
        QGraphicsItem *path = new QGraphicsEllipseItem(QRectF(-10, -10, 20, 20));
        scene.addItem(path);

        QGraphicsItem *path2 = new QGraphicsEllipseItem(QRectF(-10, -10, 20, 20));
        path2->setPos(100, 100);
        scene.addItem(path2);

        QCOMPARE(scene.itemAt(0, 0), path);
        QCOMPARE(scene.itemAt(100, 100), path2);
    }
}

void tst_QGraphicsScene::addEllipse()
{
    QGraphicsScene scene;
    QGraphicsEllipseItem *ellipse = scene.addEllipse(QRectF(-10, -10, 20, 20),
                                                     QPen(Qt::red), QBrush(Qt::blue));
    QCOMPARE(ellipse->pos(), QPointF());
    QCOMPARE(ellipse->pen(), QPen(Qt::red));
    QCOMPARE(ellipse->brush(), QBrush(Qt::blue));
    QCOMPARE(ellipse->rect(), QRectF(-10, -10, 20, 20));
    QCOMPARE(scene.itemAt(0, 0), ellipse);
    QCOMPARE(scene.itemAt(-10, -10), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(-9.9, 0), ellipse);
    QCOMPARE(scene.itemAt(-10, 10), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(0, -9.9), ellipse);
    QCOMPARE(scene.itemAt(0, 9.9), ellipse);
    QCOMPARE(scene.itemAt(10, -10), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(9.9, 0), ellipse);
    QCOMPARE(scene.itemAt(10, 10), (QGraphicsItem *)0);
}

void tst_QGraphicsScene::addLine()
{
    QGraphicsScene scene;
    QPen pen(Qt::red);
    pen.setWidthF(1.0);
    QGraphicsLineItem *line = scene.addLine(QLineF(-10, -10, 20, 20),
                                            pen);
    QCOMPARE(line->pos(), QPointF());
    QCOMPARE(line->pen(), pen);
    QCOMPARE(line->line(), QLineF(-10, -10, 20, 20));
    QCOMPARE(scene.itemAt(0, 0), line);
    QCOMPARE(scene.itemAt(-10, -10), line);
    QCOMPARE(scene.itemAt(-9.9, 0), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(-10, 10), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(0, -9.9), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(0, 9.9), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(10, -10), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(9.9, 0), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(10, 10), line);
}

void tst_QGraphicsScene::addPath()
{
    QGraphicsScene scene;
    QPainterPath p;
    p.addEllipse(QRectF(-10, -10, 20, 20));
    p.addEllipse(QRectF(-10, 20, 20, 20));

    QGraphicsPathItem *path = scene.addPath(p, QPen(Qt::red), QBrush(Qt::blue));
    QCOMPARE(path->pos(), QPointF());
    QCOMPARE(path->pen(), QPen(Qt::red));
    QCOMPARE(path->path(), p);
    QCOMPARE(path->brush(), QBrush(Qt::blue));
    QCOMPARE(scene.itemAt(0, 0), path);
    QCOMPARE(scene.itemAt(-9.9, 0), path);
    QCOMPARE(scene.itemAt(9.9, 0), path);
    QCOMPARE(scene.itemAt(0, -9.9), path);
    QCOMPARE(scene.itemAt(0, 9.9), path);
    QCOMPARE(scene.itemAt(0, 30), path);
    QCOMPARE(scene.itemAt(-9.9, 30), path);
    QCOMPARE(scene.itemAt(9.9, 30), path);
    QCOMPARE(scene.itemAt(0, 20.1), path);
    QCOMPARE(scene.itemAt(0, 39.9), path);
    QCOMPARE(scene.itemAt(-10, -10), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(10, -10), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(-10, 10), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(10, 10), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(-10, 20), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(10, 20), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(-10, 30), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(10.1, 30), (QGraphicsItem *)0);
}

void tst_QGraphicsScene::addPixmap()
{
    QGraphicsScene scene;
    QPixmap pix(":/Ash_European.jpg");
    QGraphicsPixmapItem *pixmap = scene.addPixmap(pix);

    QCOMPARE(pixmap->pos(), QPointF());
    QCOMPARE(pixmap->pixmap(), pix);
    QCOMPARE(scene.itemAt(0, 0), pixmap);
    QCOMPARE(scene.itemAt(pix.width() - 1, 0), pixmap);
    QCOMPARE(scene.itemAt(0, pix.height() - 1), pixmap);
    QCOMPARE(scene.itemAt(pix.width() - 1, pix.height() - 1), pixmap);
    QCOMPARE(scene.itemAt(-1, -1), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(pix.width() - 1, -1), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(-1, pix.height() - 1), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(pix.width(), pix.height()), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(0, pix.height()), (QGraphicsItem *)0);
    QCOMPARE(scene.itemAt(pix.width(), 0), (QGraphicsItem *)0);
}

void tst_QGraphicsScene::addRect()
{
    QGraphicsScene scene;
    QGraphicsRectItem *rect = scene.addRect(QRectF(-10, -10, 20, 20),
                                            QPen(Qt::red), QBrush(Qt::blue));
    QCOMPARE(rect->pos(), QPointF());
    QCOMPARE(rect->pen(), QPen(Qt::red));
    QCOMPARE(rect->brush(), QBrush(Qt::blue));
    QCOMPARE(rect->rect(), QRectF(-10, -10, 20, 20));
    QCOMPARE(scene.itemAt(0, 0), rect);
    QCOMPARE(scene.itemAt(-10, -10), rect);
    QCOMPARE(scene.itemAt(-9.9, 0), rect);
    QCOMPARE(scene.itemAt(-10, 10), rect);
    QCOMPARE(scene.itemAt(0, -9.9), rect);
    QCOMPARE(scene.itemAt(0, 9.9), rect);
    QCOMPARE(scene.itemAt(10, -10), rect);
    QCOMPARE(scene.itemAt(9.9, 0), rect);
    QCOMPARE(scene.itemAt(10, 10), rect);
}

void tst_QGraphicsScene::addText()
{
    QGraphicsScene scene;
    QGraphicsTextItem *text = scene.addText("Qt", QFont());
    QCOMPARE(text->pos(), QPointF());
    QCOMPARE(text->toPlainText(), QString("Qt"));
    QCOMPARE(text->font(), QFont());
}

void tst_QGraphicsScene::removeItem()
{
    QGraphicsScene scene;
    QGraphicsItem *item = scene.addRect(QRectF(0, 0, 10, 10));
    QCOMPARE(scene.itemAt(0, 0), item); // forces indexing
    scene.removeItem(item);
    QCOMPARE(scene.itemAt(0, 0), (QGraphicsItem *)0);
    delete item;

    QGraphicsItem *item2 = scene.addRect(QRectF(0, 0, 10, 10));
    item2->setFlag(QGraphicsItem::ItemIsSelectable);
    QCOMPARE(scene.itemAt(0, 0), item2);

    // Removing a selected item
    QVERIFY(scene.selectedItems().isEmpty());
    item2->setSelected(true);
    QVERIFY(scene.selectedItems().contains(item2));
    scene.removeItem(item2);
    QVERIFY(scene.selectedItems().isEmpty());

    // Removing a hovered item
    HoverItem *hoverItem = new HoverItem;
    scene.addItem(hoverItem);
    scene.setSceneRect(-50, -50, 100, 100);

    QGraphicsView view(&scene);
    view.setFixedSize(150, 150);
    view.show();
    QTest::mouseMove(view.viewport(), view.mapFromScene(hoverItem->scenePos() + QPointF(20, 20)));
    qApp->processEvents(); // update
    qApp->processEvents(); // draw
    QVERIFY(!hoverItem->isHovered);

    QTest::mouseMove(view.viewport(), view.mapFromScene(hoverItem->scenePos()));
    qApp->processEvents(); // update
    qApp->processEvents(); // draw
    QVERIFY(hoverItem->isHovered);

    scene.removeItem(hoverItem);
    hoverItem->setAcceptsHoverEvents(false);
    scene.addItem(hoverItem);
    qApp->processEvents(); // update
    qApp->processEvents(); // draw
    QVERIFY(!hoverItem->isHovered);
}

void tst_QGraphicsScene::focusItem()
{
    QGraphicsScene scene;
    QVERIFY(!scene.focusItem());
    QGraphicsItem *item = scene.addText("Qt");
    QVERIFY(!scene.focusItem());
    item->setFocus();
    QVERIFY(!scene.focusItem());
    item->setFlag(QGraphicsItem::ItemIsFocusable);
    QVERIFY(!scene.focusItem());
    item->setFocus();
    QCOMPARE(scene.focusItem(), item);

    QFocusEvent focusOut(QEvent::FocusOut);
    QApplication::sendEvent(&scene, &focusOut);

    QVERIFY(!scene.focusItem());

    QFocusEvent focusIn(QEvent::FocusIn);
    QApplication::sendEvent(&scene, &focusIn);
    QCOMPARE(scene.focusItem(), item);

    QGraphicsItem *item2 = scene.addText("Qt");
    item2->setFlag(QGraphicsItem::ItemIsFocusable);
    QCOMPARE(scene.focusItem(), item);

    item2->setFocus();
    QCOMPARE(scene.focusItem(), item2);
    item->setFocus();
    QCOMPARE(scene.focusItem(), item);

    item2->setFocus();
    QCOMPARE(scene.focusItem(), item2);
    QApplication::sendEvent(&scene, &focusOut);
    QVERIFY(!scene.hasFocus());
    QVERIFY(!scene.focusItem());
    QApplication::sendEvent(&scene, &focusIn);
    QCOMPARE(scene.focusItem(), item2);

    QApplication::sendEvent(&scene, &focusOut);

    QVERIFY(!scene.focusItem());
    scene.removeItem(item2);
    delete item2;

    QApplication::sendEvent(&scene, &focusIn);
    QVERIFY(!scene.focusItem());
}

void tst_QGraphicsScene::setFocusItem()
{
    QGraphicsScene scene;
    QGraphicsItem *item = scene.addText("Qt");
    QVERIFY(!scene.focusItem());
    QVERIFY(!scene.hasFocus());
    scene.setFocusItem(item);
    QVERIFY(!scene.hasFocus());
    QVERIFY(!scene.focusItem());
    item->setFlag(QGraphicsItem::ItemIsFocusable);

    for (int i = 0; i < 3; ++i) {
        scene.setFocusItem(item);
        QVERIFY(scene.hasFocus());
        QCOMPARE(scene.focusItem(), item);
        QVERIFY(item->hasFocus());
    }

    QGraphicsItem *item2 = scene.addText("Qt");
    item2->setFlag(QGraphicsItem::ItemIsFocusable);

    scene.setFocusItem(item2);
    QVERIFY(!item->hasFocus());
    QVERIFY(item2->hasFocus());

    scene.setFocusItem(item);
    QVERIFY(item->hasFocus());
    QVERIFY(!item2->hasFocus());

    scene.clearFocus();
    QVERIFY(!item->hasFocus());
    QVERIFY(!item2->hasFocus());

    scene.setFocus();
    QVERIFY(item->hasFocus());
    QVERIFY(!item2->hasFocus());

    scene.setFocusItem(0);
    QVERIFY(!item->hasFocus());
    QVERIFY(!item2->hasFocus());

    scene.setFocus();
    QVERIFY(!item->hasFocus());
    QVERIFY(!item2->hasFocus());
}

void tst_QGraphicsScene::mouseGrabberItem()
{
    QGraphicsScene scene;
    QVERIFY(!scene.mouseGrabberItem());

    QGraphicsItem *item = scene.addRect(QRectF(-10, -10, 20, 20));
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setZValue(1);

    QGraphicsItem *item2 = scene.addRect(QRectF(-10, -10, 20, 20));
    item2->setFlag(QGraphicsItem::ItemIsMovable);
    item2->setZValue(0);

    for (int i = 0; i < 3; ++i) {
        item->setPos(0, 0);
        item2->setPos(0, 0);
        item->setZValue((i & 1) ? 0 : 1);
        item2->setZValue((i & 1) ? 1 : 0);
        QGraphicsItem *topMostItem = (i & 1) ? item2 : item;

        QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
        pressEvent.setButton(Qt::LeftButton);
        pressEvent.setScenePos(QPointF(0, 0));
        pressEvent.setScreenPos(QPoint(100, 100));

        QApplication::sendEvent(&scene, &pressEvent);
        QCOMPARE(scene.mouseGrabberItem(), topMostItem);

        for (int i = 0; i < 1000; ++i) {
            QGraphicsSceneMouseEvent moveEvent(QEvent::GraphicsSceneMouseMove);
            moveEvent.setButtons(Qt::LeftButton);
            moveEvent.setScenePos(QPointF(i * 10, i * 10));
            moveEvent.setScreenPos(QPoint(100 + i * 10, 100 + i * 10));
            QApplication::sendEvent(&scene, &moveEvent);
            QCOMPARE(scene.mouseGrabberItem(), topMostItem);

            // Geometrical changes should not affect the mouse grabber.
            item->setZValue(rand() % 500);
            item2->setZValue(rand() % 500);
            item->setPos(rand() % 50000, rand() % 50000);
            item2->setPos(rand() % 50000, rand() % 50000);
        }

        QGraphicsSceneMouseEvent releaseEvent(QEvent::GraphicsSceneMouseRelease);
        releaseEvent.setScenePos(QPointF(10000, 10000));
        releaseEvent.setScreenPos(QPoint(1000000, 1000000));
        QApplication::sendEvent(&scene, &releaseEvent);
        QVERIFY(!scene.mouseGrabberItem());
    }

    // Structural change: deleting the mouse grabber
    item->setPos(0, 0);
    item->setZValue(1);
    item2->setPos(0, 0);
    item2->setZValue(0);
    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setScenePos(QPointF(0, 0));
    pressEvent.setScreenPos(QPoint(100, 100));

    QGraphicsSceneMouseEvent moveEvent(QEvent::GraphicsSceneMouseMove);
    moveEvent.setButtons(Qt::LeftButton);
    moveEvent.setScenePos(QPointF(0, 0));
    moveEvent.setScreenPos(QPoint(100, 100));

    QApplication::sendEvent(&scene, &pressEvent);
    QApplication::sendEvent(&scene, &moveEvent);
    QCOMPARE(scene.mouseGrabberItem(), item);
    item->setVisible(false);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);
    QApplication::sendEvent(&scene, &pressEvent);
    QCOMPARE(scene.mouseGrabberItem(), item2);
    item2->setVisible(false);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);
    QApplication::sendEvent(&scene, &moveEvent);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);
    item2->setVisible(true);
    QApplication::sendEvent(&scene, &moveEvent);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);
    QApplication::sendEvent(&scene, &pressEvent);
    QApplication::sendEvent(&scene, &moveEvent);
    QCOMPARE(scene.mouseGrabberItem(), item2);
    scene.removeItem(item2);
    delete item2;
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);
}

void tst_QGraphicsScene::hoverEvents_siblings()
{
    QGraphicsScene scene;
    QGraphicsItem *lastItem = 0;
    QList<HoverItem *> items;
    for (int i = 0; i < 15; ++i) {
        QGraphicsItem *item = new HoverItem;
        scene.addItem(item);
        items << (HoverItem *)item;
        if (lastItem) {
            item->setPos(lastItem->pos() + QPointF(sin(i / 3.0) * 17, cos(i / 3.0) * 17));
        }
        item->setZValue(i);
        lastItem = item;
    }

    QGraphicsView view(&scene);
    view.setRenderHint(QPainter::Antialiasing, true);
    view.setMinimumSize(400, 300);
    view.rotate(10);
    view.scale(1.7, 1.7);
    view.show();

    QCursor::setPos(view.mapToGlobal(QPoint(-5, -5)));

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
    mouseEvent.setScenePos(QPointF(-1000, -1000));
    QApplication::sendEvent(&scene, &mouseEvent);

    for (int j = 1; j >= 0; --j) {
        int i = j ? 0 : 14;
        forever {
            if (j)
                QVERIFY(!items.at(i)->isHovered);
            else
                QVERIFY(!items.at(i)->isHovered);
            QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
            mouseEvent.setScenePos(items.at(i)->mapToScene(0, 0));
            QApplication::sendEvent(&scene, &mouseEvent);

            qApp->processEvents(); // this posts updates from the scene to the view
            qApp->processEvents(); // which trigger a repaint here

            QVERIFY(items.at(i)->isHovered);
            if (j && i > 0)
                QVERIFY(!items.at(i - 1)->isHovered);
            if (!j && i < 14)
                QVERIFY(!items.at(i + 1)->isHovered);
            i += j ? 1 : -1;
            if ((j && i == 15) || (!j && i == -1))
                break;
        }

        QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
        mouseEvent.setScenePos(QPointF(-1000, -1000));
        QApplication::sendEvent(&scene, &mouseEvent);

        qApp->processEvents(); // this posts updates from the scene to the view
        qApp->processEvents(); // which trigger a repaint here
    }
}

void tst_QGraphicsScene::hoverEvents_parentChild()
{
    QGraphicsScene scene;
    QGraphicsItem *lastItem = 0;
    QList<HoverItem *> items;
    for (int i = 0; i < 15; ++i) {
        QGraphicsItem *item = new HoverItem;
        scene.addItem(item);
        items << (HoverItem *)item;
        if (lastItem) {
            item->setParentItem(lastItem);
            item->setPos(sin(i / 3.0) * 17, cos(i / 3.0) * 17);
        }
        lastItem = item;
    }

    QGraphicsView view(&scene);
    view.setRenderHint(QPainter::Antialiasing, true);
    view.setMinimumSize(400, 300);
    view.rotate(10);
    view.scale(1.7, 1.7);
    view.show();

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
    mouseEvent.setScenePos(QPointF(-1000, -1000));
    QApplication::sendEvent(&scene, &mouseEvent);

    for (int j = 1; j >= 0; --j) {
        int i = j ? 0 : 14;
        forever {
            if (j) {
                QVERIFY(!items.at(i)->isHovered);
            } else {
                if (i == 14)
                    QVERIFY(!items.at(13)->isHovered);
            }
            mouseEvent.setScenePos(items.at(i)->mapToScene(0, 0));
            QApplication::sendEvent(&scene, &mouseEvent);

            qApp->processEvents(); // this posts updates from the scene to the view
            qApp->processEvents(); // which trigger a repaint here

            QVERIFY(items.at(i)->isHovered);
            if (i < 14)
                QVERIFY(!items.at(i + 1)->isHovered);
            i += j ? 1 : -1;
            if ((j && i == 15) || (!j && i == -1))
                break;
        }

        mouseEvent.setScenePos(QPointF(-1000, -1000));
        QApplication::sendEvent(&scene, &mouseEvent);

        qApp->processEvents(); // this posts updates from the scene to the view
        qApp->processEvents(); // which trigger a repaint here
    }
}

void tst_QGraphicsScene::createItemGroup()
{
    QGraphicsScene scene;

    QList<QGraphicsItem *> children1;
    children1 << scene.addRect(QRectF(-10, -10, 20, 20));
    children1 << scene.addRect(QRectF(-10, -10, 20, 20));
    children1 << scene.addRect(QRectF(-10, -10, 20, 20));
    children1 << scene.addRect(QRectF(-10, -10, 20, 20));

    QList<QGraphicsItem *> children2;
    children2 << scene.addRect(QRectF(-10, -10, 20, 20));
    children2 << scene.addRect(QRectF(-10, -10, 20, 20));
    children2 << scene.addRect(QRectF(-10, -10, 20, 20));
    children2 << scene.addRect(QRectF(-10, -10, 20, 20));

    QList<QGraphicsItem *> children3;
    children3 << scene.addRect(QRectF(-10, -10, 20, 20));
    children3 << scene.addRect(QRectF(-10, -10, 20, 20));
    children3 << scene.addRect(QRectF(-10, -10, 20, 20));
    children3 << scene.addRect(QRectF(-10, -10, 20, 20));

    // All items in children1 are children of parent1
    QGraphicsItem *parent1 = scene.addRect(QRectF(-10, -10, 20, 20));
    foreach (QGraphicsItem *item, children1)
        item->setParentItem(parent1);

    QGraphicsItemGroup *group = scene.createItemGroup(children1);
    QCOMPARE(group->parentItem(), parent1);
    QCOMPARE(children1.first()->parentItem(), group);
    scene.destroyItemGroup(group);
    QCOMPARE(children1.first()->parentItem(), parent1);
    group = scene.createItemGroup(children1);
    QCOMPARE(group->parentItem(), parent1);
    QCOMPARE(children1.first()->parentItem(), group);
    scene.destroyItemGroup(group);
    QCOMPARE(children1.first()->parentItem(), parent1);

    // All items in children2 are children of parent2
    QGraphicsItem *parent2 = scene.addRect(QRectF(-10, -10, 20, 20));
    foreach (QGraphicsItem *item, children2)
        item->setParentItem(parent2);

    // Now make parent2 a child of parent1, so all children2 are also children
    // of parent1.
    parent2->setParentItem(parent1);

    // The children2 group should still have parent2 as their common ancestor.
    group = scene.createItemGroup(children2);
    QCOMPARE(group->parentItem(), parent2);
    QCOMPARE(children2.first()->parentItem(), group);
    scene.destroyItemGroup(group);
    QCOMPARE(children2.first()->parentItem(), parent2);

    // But the set of both children2 and children1 share only parent1.
    group = scene.createItemGroup(children2 + children1);
    QCOMPARE(group->parentItem(), parent1);
    QCOMPARE(children1.first()->parentItem(), group);
    QCOMPARE(children2.first()->parentItem(), group);
    scene.destroyItemGroup(group);
    QCOMPARE(children1.first()->parentItem(), parent1);
    QCOMPARE(children2.first()->parentItem(), parent1);

    // Fixup the parent-child chain
    foreach (QGraphicsItem *item, children2)
        item->setParentItem(parent2);

    // These share no common parent
    group = scene.createItemGroup(children3);
    QCOMPARE(group->parentItem(), (QGraphicsItem *)0);
    scene.destroyItemGroup(group);

    // Make children3 children of parent3
    QGraphicsItem *parent3 = scene.addRect(QRectF(-10, -10, 20, 20));
    foreach (QGraphicsItem *item, children3)
        item->setParentItem(parent3);

    // These should have parent3 as a parent
    group = scene.createItemGroup(children3);
    QCOMPARE(group->parentItem(), parent3);
    scene.destroyItemGroup(group);

    // Now make them all children of parent1
    parent3->setParentItem(parent1);

    group = scene.createItemGroup(children3);
    QCOMPARE(group->parentItem(), parent3);
    scene.destroyItemGroup(group);

    group = scene.createItemGroup(children2);
    QCOMPARE(group->parentItem(), parent2);
    scene.destroyItemGroup(group);

    group = scene.createItemGroup(children1);
    QCOMPARE(group->parentItem(), parent1);
    scene.destroyItemGroup(group);
}

class EventTester : public QGraphicsEllipseItem
{
public:
    EventTester()
        : QGraphicsEllipseItem(QRectF(-10, -10, 20, 20)), ignoreMouse(false)
    { }

    bool ignoreMouse;
    QList<QEvent::Type> eventTypes;

protected:
    bool sceneEvent(QEvent *event)
    {
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneMouseMove:
        case QEvent::GraphicsSceneMouseRelease:
            if (ignoreMouse) {
                event->ignore();
                return true;
            }
        default:
            break;
        }

        eventTypes << QEvent::Type(event->type());
        return QGraphicsEllipseItem::sceneEvent(event);
    }
};

void tst_QGraphicsScene::mouseEventPropagation()
{
    EventTester *a = new EventTester;
    EventTester *b = new EventTester;
    EventTester *c = new EventTester;
    EventTester *d = new EventTester;
    b->setParentItem(a);
    c->setParentItem(b);
    d->setParentItem(c);

    a->setFlag(QGraphicsItem::ItemIsMovable);
    b->setFlag(QGraphicsItem::ItemIsMovable);
    c->setFlag(QGraphicsItem::ItemIsMovable);
    d->setFlag(QGraphicsItem::ItemIsMovable);

    // scene -> a -> b -> c -> d
    QGraphicsScene scene;
    scene.addItem(a);

    // Prepare some events
    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setScenePos(QPointF(0, 0));
    QGraphicsSceneMouseEvent moveEvent(QEvent::GraphicsSceneMouseMove);
    moveEvent.setButton(Qt::LeftButton);
    moveEvent.setScenePos(QPointF(0, 0));
    QGraphicsSceneMouseEvent releaseEvent(QEvent::GraphicsSceneMouseRelease);
    releaseEvent.setButton(Qt::LeftButton);
    releaseEvent.setScenePos(QPointF(0, 0));

    // Send a press
    QApplication::sendEvent(&scene, &pressEvent);
    QCOMPARE(d->eventTypes.size(), 1);
    QCOMPARE(d->eventTypes.at(0), QEvent::GraphicsSceneMousePress);
    QCOMPARE(c->eventTypes.size(), 0);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(a->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)d);

    // Send a move
    QApplication::sendEvent(&scene, &moveEvent);
    QCOMPARE(d->eventTypes.size(), 2);
    QCOMPARE(d->eventTypes.at(1), QEvent::GraphicsSceneMouseMove);
    QCOMPARE(c->eventTypes.size(), 0);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(a->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)d);

    // Send a release
    QApplication::sendEvent(&scene, &releaseEvent);
    QCOMPARE(d->eventTypes.size(), 3);
    QCOMPARE(d->eventTypes.at(2), QEvent::GraphicsSceneMouseRelease);
    QCOMPARE(c->eventTypes.size(), 0);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(a->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);

    d->setAcceptedMouseButtons(Qt::RightButton);

    // Send a press
    QApplication::sendEvent(&scene, &pressEvent);
    QCOMPARE(d->eventTypes.size(), 3);
    QCOMPARE(c->eventTypes.size(), 1);
    QCOMPARE(c->eventTypes.at(0), QEvent::GraphicsSceneMousePress);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(a->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)c);

    // Send another press, with a button that isn't actually accepted
    QApplication::sendEvent(&scene, &pressEvent);
    pressEvent.setButton(Qt::RightButton);
    QCOMPARE(d->eventTypes.size(), 3);
    QCOMPARE(c->eventTypes.size(), 2);
    QCOMPARE(c->eventTypes.at(1), QEvent::GraphicsSceneMousePress);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(a->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)c);

    // Send a move
    QApplication::sendEvent(&scene, &moveEvent);
    QCOMPARE(d->eventTypes.size(), 3);
    QCOMPARE(c->eventTypes.size(), 3);
    QCOMPARE(c->eventTypes.at(2), QEvent::GraphicsSceneMouseMove);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(a->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)c);

    // Send a release
    QApplication::sendEvent(&scene, &releaseEvent);
    QCOMPARE(d->eventTypes.size(), 3);
    QCOMPARE(c->eventTypes.size(), 4);
    QCOMPARE(c->eventTypes.at(3), QEvent::GraphicsSceneMouseRelease);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(a->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);

    // Disabled items eat events. c should not get this.
    d->setEnabled(false);
    d->setAcceptedMouseButtons(Qt::RightButton);

    // Send a right press. This disappears in d.
    QApplication::sendEvent(&scene, &pressEvent);
    QCOMPARE(d->eventTypes.size(), 3);
    QCOMPARE(c->eventTypes.size(), 4);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(a->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);

    // Send a left press. This goes to c.
    pressEvent.setButton(Qt::LeftButton);
    QApplication::sendEvent(&scene, &pressEvent);
    QCOMPARE(d->eventTypes.size(), 3);
    QCOMPARE(c->eventTypes.size(), 5);
    QCOMPARE(c->eventTypes.last(), QEvent::GraphicsSceneMousePress);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(a->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)c);

    // Clicking outside the items removes the mouse grabber
}

void tst_QGraphicsScene::mouseEventPropagation_ignore()
{
    EventTester *a = new EventTester;
    EventTester *b = new EventTester;
    EventTester *c = new EventTester;
    EventTester *d = new EventTester;
    b->setParentItem(a);
    c->setParentItem(b);
    d->setParentItem(c);

    // scene -> a -> b -> c -> d
    QGraphicsScene scene;
    scene.addItem(a);

    // Prepare some events
    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setScenePos(QPointF(0, 0));

    b->ignoreMouse = true;
    c->ignoreMouse = true;
    d->ignoreMouse = true;

    QApplication::sendEvent(&scene, &pressEvent);
    QCOMPARE(a->eventTypes.size(), 1);
    QCOMPARE(a->eventTypes.first(), QEvent::GraphicsSceneMousePress);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(c->eventTypes.size(), 0);
    QCOMPARE(d->eventTypes.size(), 0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)a);

    a->ignoreMouse = true;

    QApplication::sendEvent(&scene, &pressEvent);
    QCOMPARE(a->eventTypes.size(), 1);
    QCOMPARE(b->eventTypes.size(), 0);
    QCOMPARE(c->eventTypes.size(), 0);
    QCOMPARE(d->eventTypes.size(), 0);

    QVERIFY(!pressEvent.isAccepted());
}

void tst_QGraphicsScene::mouseEventPropagation_focus()
{
    EventTester *a = new EventTester;
    EventTester *b = new EventTester;
    EventTester *c = new EventTester;
    EventTester *d = new EventTester;
    b->setParentItem(a);
    c->setParentItem(b);
    d->setParentItem(c);

    a->setFlag(QGraphicsItem::ItemIsMovable);
    b->setFlag(QGraphicsItem::ItemIsMovable);
    c->setFlag(QGraphicsItem::ItemIsMovable);
    d->setFlag(QGraphicsItem::ItemIsMovable);

    // scene -> a -> b -> c -> d
    QGraphicsScene scene;
    scene.addItem(a);

    // Prepare some events
    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setScenePos(QPointF(0, 0));

    a->setFlag(QGraphicsItem::ItemIsFocusable);
    QVERIFY(!a->hasFocus());

    QApplication::sendEvent(&scene, &pressEvent);

    QVERIFY(a->hasFocus());
    QCOMPARE(a->eventTypes.size(), 1);
    QCOMPARE(a->eventTypes.first(), QEvent::FocusIn);
    QCOMPARE(d->eventTypes.size(), 1);
    QCOMPARE(d->eventTypes.first(), QEvent::GraphicsSceneMousePress);
}

void tst_QGraphicsScene::mouseEventPropagation_doubleclick()
{
    EventTester *a = new EventTester;
    EventTester *b = new EventTester;
    a->setPos(-50, 0);
    b->setPos(50, 0);

    QGraphicsScene scene;
    scene.addItem(a);
    scene.addItem(b);

    // Prepare some events
    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setScenePos(QPointF(0, 0));
    QGraphicsSceneMouseEvent doubleClickEvent(QEvent::GraphicsSceneMouseDoubleClick);
    doubleClickEvent.setButton(Qt::LeftButton);
    doubleClickEvent.setScenePos(QPointF(0, 0));
    QGraphicsSceneMouseEvent releaseEvent(QEvent::GraphicsSceneMouseRelease);
    releaseEvent.setButton(Qt::LeftButton);
    releaseEvent.setScenePos(QPointF(0, 0));

    // Send press to A
    pressEvent.setScenePos(a->mapToScene(0, 0));
    QApplication::sendEvent(&scene, &pressEvent);
    QCOMPARE(a->eventTypes.size(), 1);
    QCOMPARE(a->eventTypes.last(), QEvent::GraphicsSceneMousePress);

    // Send release to A
    releaseEvent.setScenePos(a->mapToScene(0, 0));
    QApplication::sendEvent(&scene, &releaseEvent);
    QCOMPARE(a->eventTypes.size(), 2);
    QCOMPARE(a->eventTypes.last(), QEvent::GraphicsSceneMouseRelease);

    // Send doubleclick to B
    doubleClickEvent.setScenePos(b->mapToScene(0, 0));
    QApplication::sendEvent(&scene, &doubleClickEvent);
    QCOMPARE(a->eventTypes.size(), 2);
    QCOMPARE(b->eventTypes.size(), 1);
    QCOMPARE(b->eventTypes.last(), QEvent::GraphicsSceneMousePress);

    // Send release to B
    releaseEvent.setScenePos(b->mapToScene(0, 0));
    QApplication::sendEvent(&scene, &releaseEvent);
    QCOMPARE(a->eventTypes.size(), 2);
    QCOMPARE(b->eventTypes.size(), 2);
    QCOMPARE(b->eventTypes.last(), QEvent::GraphicsSceneMouseRelease);
}

class Scene : public QGraphicsScene
{
public:
    QList<QPointF> mouseMovePoints;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        mouseMovePoints << event->scenePos();
    }
};

void tst_QGraphicsScene::mouseEventPropagation_mouseMove()
{
    Scene scene;
    scene.addRect(QRectF(5, 0, 12, 12));
    scene.addRect(QRectF(15, 0, 12, 12))->setAcceptsHoverEvents(true);
    for (int i = 0; i < 30; ++i) {
        QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMouseMove);
        event.setScenePos(QPointF(i, 5));
        QApplication::sendEvent(&scene, &event);
    }

    QCOMPARE(scene.mouseMovePoints.size(), 17);
    for (int i = 0; i < 15; ++i)
        QCOMPARE(scene.mouseMovePoints.at(i), QPointF(i, 5));
    QCOMPARE(scene.mouseMovePoints.at(15), QPointF(28, 5));
    QCOMPARE(scene.mouseMovePoints.at(16), QPointF(29, 5));
}

class DndTester : public QGraphicsEllipseItem
{
public:
    DndTester(const QRectF &rect)
        : QGraphicsEllipseItem(rect), lastEvent(0),
          ignoresDragEnter(false), ignoresDragMove(false)

    {
    }

    ~DndTester()
    {
        delete lastEvent;
    }

    QGraphicsSceneDragDropEvent *lastEvent;
    QList<QEvent::Type> eventList;
    bool ignoresDragEnter;
    bool ignoresDragMove;

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event)
    {
        storeLastEvent(event);
        event->setAccepted(!ignoresDragEnter);
        if (!ignoresDragEnter)
            event->setDropAction(Qt::IgnoreAction);
        eventList << event->type();
    }

    void dragMoveEvent(QGraphicsSceneDragDropEvent *event)
    {
        storeLastEvent(event);
        event->setAccepted(!ignoresDragMove);
        eventList << event->type();
    }

    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
    {
        storeLastEvent(event);
        eventList << event->type();
    }

    void dropEvent(QGraphicsSceneDragDropEvent *event)
    {
        storeLastEvent(event);
        eventList << event->type();
    }

private:
    void storeLastEvent(QGraphicsSceneDragDropEvent *event)
    {
        delete lastEvent;
        lastEvent = new QGraphicsSceneDragDropEvent(event->type());
        lastEvent->setScenePos(event->scenePos());
        lastEvent->setScreenPos(event->screenPos());
        lastEvent->setButtons(event->buttons());
        lastEvent->setModifiers(event->modifiers());
        lastEvent->setPossibleActions(event->possibleActions());
        lastEvent->setProposedAction(event->proposedAction());
        lastEvent->setDropAction(event->dropAction());
        lastEvent->setMimeData(event->mimeData());
        lastEvent->setWidget(event->widget());
        lastEvent->setSource(event->source());
    }
};

void tst_QGraphicsScene::dragAndDrop_simple()
{
    DndTester *item = new DndTester(QRectF(-10, -10, 20, 20));

    QGraphicsScene scene;
    scene.addItem(item);

    QGraphicsView view(&scene);
    view.setFixedSize(100, 100);

    QMimeData mimeData;

    // Initial drag enter for the scene
    QDragEnterEvent dragEnter(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
    QApplication::sendEvent(view.viewport(), &dragEnter);
    QVERIFY(dragEnter.isAccepted());
    QCOMPARE(dragEnter.dropAction(), Qt::CopyAction);

    {
        // Move outside the item
        QDragMoveEvent dragMove(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(!dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::CopyAction);
    }
    {
        // Move inside the item without setAcceptDrops
        QDragMoveEvent dragMove(view.mapFromScene(item->scenePos()), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(!dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::CopyAction);
        QCOMPARE(item->eventList.size(), 0);
    }
    item->setAcceptDrops(true);
    {
        // Move inside the item with setAcceptDrops
        QDragMoveEvent dragMove(view.mapFromScene(item->scenePos()), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::IgnoreAction);
        QCOMPARE(item->eventList.size(), 2);
        QCOMPARE(item->eventList.at(0), QEvent::GraphicsSceneDragEnter);
        QCOMPARE(item->eventList.at(1), QEvent::GraphicsSceneDragMove);
        QCOMPARE(item->lastEvent->screenPos(), view.mapToGlobal(dragMove.pos()));
        QCOMPARE(item->lastEvent->scenePos(), view.mapToScene(dragMove.pos()));
        QVERIFY(item->lastEvent->isAccepted());
        QCOMPARE(item->lastEvent->dropAction(), Qt::IgnoreAction);
    }
    {
        // Another move inside the item
        QDragMoveEvent dragMove(view.mapFromScene(item->mapToScene(5, 5)), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::IgnoreAction);
        QCOMPARE(item->eventList.size(), 3);
        QCOMPARE(item->eventList.at(2), QEvent::GraphicsSceneDragMove);
        QCOMPARE(item->lastEvent->screenPos(), view.mapToGlobal(dragMove.pos()));
        QCOMPARE(item->lastEvent->scenePos(), view.mapToScene(dragMove.pos()));
        QVERIFY(item->lastEvent->isAccepted());
        QCOMPARE(item->lastEvent->dropAction(), Qt::IgnoreAction);
    }
    {
        // Move outside the item
        QDragMoveEvent dragMove(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(!dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::CopyAction);
        QCOMPARE(item->eventList.size(), 4);
        QCOMPARE(item->eventList.at(3), QEvent::GraphicsSceneDragLeave);
        QCOMPARE(item->lastEvent->screenPos(), view.mapToGlobal(dragMove.pos()));
        QCOMPARE(item->lastEvent->scenePos(), view.mapToScene(dragMove.pos()));
        QVERIFY(item->lastEvent->isAccepted());
        QCOMPARE(item->lastEvent->dropAction(), Qt::CopyAction);
    }
    {
        // Move inside the item again
        QDragMoveEvent dragMove(view.mapFromScene(item->scenePos()), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::IgnoreAction);
        QCOMPARE(item->eventList.size(), 6);
        QCOMPARE(item->eventList.at(4), QEvent::GraphicsSceneDragEnter);
        QCOMPARE(item->eventList.at(5), QEvent::GraphicsSceneDragMove);
        QCOMPARE(item->lastEvent->screenPos(), view.mapToGlobal(dragMove.pos()));
        QCOMPARE(item->lastEvent->scenePos(), view.mapToScene(dragMove.pos()));
        QVERIFY(item->lastEvent->isAccepted());
        QCOMPARE(item->lastEvent->dropAction(), Qt::IgnoreAction);
    }
    {
        // Drop inside the item
        QDropEvent drop(view.mapFromScene(item->scenePos()), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &drop);
        QVERIFY(drop.isAccepted());
        QCOMPARE(drop.dropAction(), Qt::CopyAction);
        QCOMPARE(item->eventList.size(), 7);
        QCOMPARE(item->eventList.at(6), QEvent::GraphicsSceneDrop);
        QCOMPARE(item->lastEvent->screenPos(), view.mapToGlobal(drop.pos()));
        QCOMPARE(item->lastEvent->scenePos(), view.mapToScene(drop.pos()));
        QVERIFY(item->lastEvent->isAccepted());
        QCOMPARE(item->lastEvent->dropAction(), Qt::CopyAction);
    }
}

void tst_QGraphicsScene::dragAndDrop_disabledOrInvisible()
{
    DndTester *item = new DndTester(QRectF(-10, -10, 20, 20));
    item->setAcceptDrops(true);

    QGraphicsScene scene;
    scene.addItem(item);

    QGraphicsView view(&scene);
    view.setFixedSize(100, 100);

    QMimeData mimeData;

    // Initial drag enter for the scene
    QDragEnterEvent dragEnter(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
    QApplication::sendEvent(view.viewport(), &dragEnter);
    QVERIFY(dragEnter.isAccepted());
    QCOMPARE(dragEnter.dropAction(), Qt::CopyAction);
    {
        // Move inside the item
        QDragMoveEvent dragMove(view.mapFromScene(item->scenePos()), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::IgnoreAction);
        QCOMPARE(item->eventList.size(), 2);
        QCOMPARE(item->eventList.at(0), QEvent::GraphicsSceneDragEnter);
        QCOMPARE(item->eventList.at(1), QEvent::GraphicsSceneDragMove);
    }
    {
        // Move outside the item
        QDragMoveEvent dragMove(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(!dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::CopyAction);
        QCOMPARE(item->eventList.size(), 3);
        QCOMPARE(item->eventList.at(2), QEvent::GraphicsSceneDragLeave);
    }

    // Now disable the item
    item->setEnabled(false);
    QVERIFY(!item->isEnabled());
    QVERIFY(item->isVisible());

    {
        // Move inside the item
        QDragMoveEvent dragMove(view.mapFromScene(item->scenePos()), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(!dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::CopyAction);
        QCOMPARE(item->eventList.size(), 3);
        QCOMPARE(item->eventList.at(2), QEvent::GraphicsSceneDragLeave);
    }

    // Reenable it, and make it invisible
    item->setEnabled(true);
    item->setVisible(false);
    QVERIFY(item->isEnabled());
    QVERIFY(!item->isVisible());

    {
        // Move inside the item
        QDragMoveEvent dragMove(view.mapFromScene(item->scenePos()), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(!dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::CopyAction);
        QCOMPARE(item->eventList.size(), 3);
        QCOMPARE(item->eventList.at(2), QEvent::GraphicsSceneDragLeave);
    }
    
    // Dummy drop event to keep the Mac from crashing.
    QDropEvent dropEvent(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
    QApplication::sendEvent(view.viewport(), &dropEvent);
}

void tst_QGraphicsScene::dragAndDrop_propagate()
{
    DndTester *item1 = new DndTester(QRectF(-10, -10, 20, 20));
    DndTester *item2 = new DndTester(QRectF(0, 0, 20, 20));
    item1->setAcceptDrops(true);
    item2->setAcceptDrops(true);
    item2->ignoresDragMove = true;
    item2->ignoresDragEnter = false;
    item2->setZValue(1);

    item1->setData(0, "item1");
    item2->setData(0, "item2");

    QGraphicsScene scene;
    scene.addItem(item1);
    scene.addItem(item2);

    QGraphicsView view(&scene);
    view.setFixedSize(100, 100);

    QMimeData mimeData;

    // Initial drag enter for the scene
    QDragEnterEvent dragEnter(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
    QApplication::sendEvent(view.viewport(), &dragEnter);
    QVERIFY(dragEnter.isAccepted());
    QCOMPARE(dragEnter.dropAction(), Qt::CopyAction);

    {
        // Move outside the items
        QDragMoveEvent dragMove(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(!dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::CopyAction);
        QVERIFY(item1->eventList.isEmpty());
        QVERIFY(item2->eventList.isEmpty());
    }
    {
        // Move inside item1
        QDragMoveEvent dragMove(view.mapFromScene(-5, -5), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::IgnoreAction);
        QCOMPARE(item1->eventList.size(), 2);
        QCOMPARE(item1->eventList.at(0), QEvent::GraphicsSceneDragEnter);
        QCOMPARE(item1->eventList.at(1), QEvent::GraphicsSceneDragMove);
    }
    {
        // Move into the intersection item1-item2
        QDragMoveEvent dragMove(view.mapFromScene(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::IgnoreAction);
        QCOMPARE(item1->eventList.size(), 5);
        QCOMPARE(item1->eventList.at(2), QEvent::GraphicsSceneDragLeave);
        QCOMPARE(item1->eventList.at(3), QEvent::GraphicsSceneDragEnter);
        QCOMPARE(item1->eventList.at(4), QEvent::GraphicsSceneDragMove);
        QCOMPARE(item2->eventList.size(), 3);
        QCOMPARE(item2->eventList.at(0), QEvent::GraphicsSceneDragEnter);
        QCOMPARE(item2->eventList.at(1), QEvent::GraphicsSceneDragMove);
        QCOMPARE(item2->eventList.at(2), QEvent::GraphicsSceneDragLeave);
    }
    {
        // Move into the item2
        QDragMoveEvent dragMove(view.mapFromScene(15, 15), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(!dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::CopyAction);
        QCOMPARE(item1->eventList.size(), 6);
        QCOMPARE(item1->eventList.at(5), QEvent::GraphicsSceneDragLeave);
        QCOMPARE(item2->eventList.size(), 6);
        QCOMPARE(item2->eventList.at(3), QEvent::GraphicsSceneDragEnter);
        QCOMPARE(item2->eventList.at(4), QEvent::GraphicsSceneDragMove);
        QCOMPARE(item2->eventList.at(5), QEvent::GraphicsSceneDragLeave);
    }
    {
        // Move back into the intersection item1-item2
        QDragMoveEvent dragMove(view.mapFromScene(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &dragMove);
        QVERIFY(dragMove.isAccepted());
        QCOMPARE(dragMove.dropAction(), Qt::IgnoreAction);
        QCOMPARE(item1->eventList.size(), 8);
        QCOMPARE(item1->eventList.at(6), QEvent::GraphicsSceneDragEnter);
        QCOMPARE(item1->eventList.at(7), QEvent::GraphicsSceneDragMove);
        QCOMPARE(item2->eventList.size(), 9);
        QCOMPARE(item2->eventList.at(6), QEvent::GraphicsSceneDragEnter);
        QCOMPARE(item2->eventList.at(7), QEvent::GraphicsSceneDragMove);
        QCOMPARE(item2->eventList.at(8), QEvent::GraphicsSceneDragLeave);
    }
    {
        // Drop on the intersection item1-item2
        QDropEvent drop(view.mapFromScene(5, 5), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
        QApplication::sendEvent(view.viewport(), &drop);
        QVERIFY(drop.isAccepted());
        QCOMPARE(drop.dropAction(), Qt::CopyAction);

        QCOMPARE(item1->eventList.size(), 9);
        QCOMPARE(item1->eventList.at(8), QEvent::GraphicsSceneDrop);
        QCOMPARE(item2->eventList.size(), 9);
    }

    // Dummy drop event to keep the Mac from crashing.
    QDropEvent dropEvent(QPoint(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, 0);
    QApplication::sendEvent(view.viewport(), &dropEvent);
}

void tst_QGraphicsScene::render_data()
{
    QTest::addColumn<QRectF>("targetRect");
    QTest::addColumn<QRectF>("sourceRect");
    QTest::addColumn<Qt::AspectRatioMode>("aspectRatioMode");
    QTest::addColumn<QMatrix>("matrix");

    QTest::newRow("all-all-untransformed") << QRectF() << QRectF()
                                           << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("all-topleft-untransformed") << QRectF(0, 0, 150, 150)
                                               << QRectF() << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("all-topright-untransformed") << QRectF(150, 0, 150, 150)
                                                << QRectF() << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("all-bottomleft-untransformed") << QRectF(0, 150, 150, 150)
                                                  << QRectF() << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("all-bottomright-untransformed") << QRectF(150, 150, 150, 150)
                                                   << QRectF() << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("topleft-all-untransformed") << QRectF() << QRectF(-10, -10, 10, 10)
                                               << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("topright-all-untransformed") << QRectF() << QRectF(0, -10, 10, 10)
                                                << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("bottomleft-all-untransformed") << QRectF() << QRectF(-10, 0, 10, 10)
                                                  << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("bottomright-all-untransformed") << QRectF() << QRectF(0, 0, 10, 10)
                                                   << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("topleft-topleft-untransformed") << QRectF(0, 0, 150, 150) << QRectF(-10, -10, 10, 10)
                                                   << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("topright-topleft-untransformed") << QRectF(150, 0, 150, 150) << QRectF(-10, -10, 10, 10)
                                                    << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("bottomleft-topleft-untransformed") << QRectF(0, 150, 150, 150) << QRectF(-10, -10, 10, 10)
                                                      << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("bottomright-topleft-untransformed") << QRectF(150, 150, 150, 150) << QRectF(-10, -10, 10, 10)
                                                       << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("top-topleft-untransformed") << QRectF(0, 0, 300, 150) << QRectF(-10, -10, 10, 10)
                                               << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("bottom-topleft-untransformed") << QRectF(0, 150, 300, 150) << QRectF(-10, -10, 10, 10)
                                                  << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("left-topleft-untransformed") << QRectF(0, 0, 150, 300) << QRectF(-10, -10, 10, 10)
                                                << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("right-topleft-untransformed") << QRectF(150, 0, 150, 300) << QRectF(-10, -10, 10, 10)
                                                 << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("top-bottomright-untransformed") << QRectF(0, 0, 300, 150) << QRectF(0, 0, 10, 10)
                                                   << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("bottom-bottomright-untransformed") << QRectF(0, 150, 300, 150) << QRectF(0, 0, 10, 10)
                                                      << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("left-bottomright-untransformed") << QRectF(0, 0, 150, 300) << QRectF(0, 0, 10, 10)
                                                    << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("right-bottomright-untransformed") << QRectF(150, 0, 150, 300) << QRectF(0, 0, 10, 10)
                                                     << Qt::IgnoreAspectRatio << QMatrix();
    QTest::newRow("all-all-45-deg-right") << QRectF() << QRectF()
                                          << Qt::IgnoreAspectRatio << QMatrix().rotate(-45);
    QTest::newRow("all-all-45-deg-left") << QRectF() << QRectF()
                                         << Qt::IgnoreAspectRatio << QMatrix().rotate(45);
    QTest::newRow("all-all-scale-2x") << QRectF() << QRectF()
                                      << Qt::IgnoreAspectRatio << QMatrix().scale(2, 2);
    QTest::newRow("all-all-translate-50-0") << QRectF() << QRectF()
                                            << Qt::IgnoreAspectRatio << QMatrix().translate(50, 0);
    QTest::newRow("all-all-translate-0-50") << QRectF() << QRectF()
                                            << Qt::IgnoreAspectRatio << QMatrix().translate(0, 50);
}

void tst_QGraphicsScene::render()
{
    QFETCH(QRectF, targetRect);
    QFETCH(QRectF, sourceRect);
    QFETCH(Qt::AspectRatioMode, aspectRatioMode);
    QFETCH(QMatrix, matrix);

    QGraphicsScene scene;
    scene.addEllipse(QRectF(-10, -10, 20, 20), QPen(Qt::black), QBrush(Qt::white));
    scene.addEllipse(QRectF(-2, -7, 4, 4), QPen(Qt::black), QBrush(Qt::yellow))->setZValue(1);
    scene.setSceneRect(scene.itemsBoundingRect());

    QImage bigImage(300, 300, QImage::Format_RGB32);
    bigImage.fill(0);
    QPainter painter(&bigImage);
    painter.setPen(Qt::lightGray);
    for (int i = 0; i <= 300; i += 25) {
        painter.drawLine(0, i, 300, i);
        painter.drawLine(i, 0, i, 300);
    }
    painter.setPen(QPen(Qt::darkGray, 2));
    painter.drawLine(0, 150, 300, 150);
    painter.drawLine(150, 0, 150, 300);
    painter.setMatrix(matrix);
    scene.render(&painter, targetRect, sourceRect, aspectRatioMode);
    painter.end();

    QString fileName = QString("testData/render/%1.png").arg(QTest::currentDataTag());
    QImage original(fileName);
    QVERIFY(!original.isNull());
    
    // Compare
    int wrongPixels = 0;
    for (int y = 0; y < original.height(); ++y) {
        for (int x = 0; x < original.width(); ++x) {
            if (bigImage.pixel(x, y) != original.pixel(x, y))
                ++wrongPixels;
        }
    }

    // This is a pixmap compare test - because of rounding errors on diverse
    // platforms, and especially because tests are compiled in release mode,
    // we set a 95% acceptance threshold for comparing images. This number may
    // have to be adjusted if this test fails.
    qreal threshold = 0.95;
    qreal similarity = (1 - (wrongPixels / qreal(original.width() * original.height())));
    if (similarity < threshold) {
#if 1
        // fail
        QLabel *expectedLabel = new QLabel;
        expectedLabel->setPixmap(QPixmap::fromImage(original));

        QLabel *newLabel = new QLabel;
        newLabel->setPixmap(QPixmap::fromImage(bigImage));

        QGridLayout *gridLayout = new QGridLayout;
        gridLayout->addWidget(new QLabel(tr("MISMATCH: %1").arg(QTest::currentDataTag())), 0, 0, 1, 2);
        gridLayout->addWidget(new QLabel(tr("Current")), 1, 0);
        gridLayout->addWidget(new QLabel(tr("Expected")), 1, 1);
        gridLayout->addWidget(expectedLabel, 2, 1);
        gridLayout->addWidget(newLabel, 2, 0);

        QWidget widget;
        widget.setLayout(gridLayout);
        widget.show();

        QTestEventLoop::instance().enterLoop(1);

        QFAIL("Images are not identical.");
#else
        // generate
        qDebug() << "Updating" << QTest::currentDataTag() << ":" << bigImage.save(fileName, "png");
#endif
    }
}

void tst_QGraphicsScene::contextMenuEvent()
{
    QGraphicsScene scene;
    EventTester *item = new EventTester;
    scene.addItem(item);
    item->setFlag(QGraphicsItem::ItemIsFocusable);
    item->setFocus();

    QVERIFY(item->hasFocus());
    QVERIFY(scene.hasFocus());

    QGraphicsView view(&scene);
    view.show();
    view.centerOn(item);

    {
        QContextMenuEvent event(QContextMenuEvent::Keyboard, view.viewport()->rect().center(),
                                view.mapToGlobal(view.viewport()->rect().center()));
        QApplication::sendEvent(view.viewport(), &event);
        QCOMPARE(item->eventTypes.last(), QEvent::GraphicsSceneContextMenu);
    }
}

void tst_QGraphicsScene::update()
{
    QGraphicsScene scene;

    QGraphicsRectItem *rect = new QGraphicsRectItem(0, 0, 100, 100);
    scene.addItem(rect);
    rect->setPos(-100, -100);

    // This function forces indexing
    scene.itemAt(0, 0);

    qRegisterMetaType<QList<QRectF> >("QList<QRectF>");
    QSignalSpy spy(&scene, SIGNAL(changed(QList<QRectF>)));

    // When deleted, the item will lazy-remove itself
    delete rect;

    // This function forces a purge, which will post an update signal
    scene.itemAt(0, 0);

    // This will process the pending update
    QApplication::instance()->processEvents();

    // Check that the update region is correct
    QCOMPARE(spy.count(), 1);
    QCOMPARE(qVariantValue<QList<QRectF> >(spy.at(0).at(0)).at(0),
             QRectF(-100, -100, 200, 200));
}

void tst_QGraphicsScene::views()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);

    QVERIFY(scene.views().size() == 1);
    QVERIFY(scene.views().at(0) == &view);

    QGraphicsView view1(&scene);
    QVERIFY(scene.views().size() == 2);
    QVERIFY(scene.views().at(0) == &view1 || scene.views().at(1) == &view1);

    view.setScene(0);
    QVERIFY(scene.views().size() == 1);
    QVERIFY(scene.views().at(0) == &view1);    
}

class CustomScene : public QGraphicsScene
{
public:
    CustomScene() : gotTimerEvent(false)
    { startTimer(10); }
    
    bool gotTimerEvent;
protected:
    void timerEvent(QTimerEvent *)
    {
        gotTimerEvent = true;
    }
};

void tst_QGraphicsScene::event()
{
    // Test that QGraphicsScene properly propagates events to QObject.
    CustomScene scene;
    QTestEventLoop::instance().enterLoop(1);
    QVERIFY(scene.gotTimerEvent);
}

void tst_QGraphicsScene::task139710_bspTreeCrash()
{
    // create a scene with 2000 items
    QGraphicsScene scene(0, 0, 1000, 1000);

    for (int i = 0; i < 2; ++i) {
        // trigger delayed item indexing
        qApp->processEvents();
        scene.setSceneRect(0, 0, 10000, 10000);

        // delete all items in the scene - pointers are now likely to be recycled
        foreach (QGraphicsItem *item, scene.items()) {
            scene.removeItem(item);
            delete item;
        }

        // add 1000 more items - the BSP tree is now resized
        for (int i = 0; i < 1000; ++i) {
            QGraphicsRectItem *item = scene.addRect(QRectF(0, 0, 200, 200));
            item->setPos(qrand() % 10000, qrand() % 10000);
        }

        // trigger delayed item indexing for the first 1000 items
        qApp->processEvents();

        // add 1000 more items - the BSP tree is now resized
        for (int i = 0; i < 1000; ++i) {
            QGraphicsRectItem *item = scene.addRect(QRectF(0, 0, 200, 200));
            item->setPos(qrand() % 10000, qrand() % 10000);
        }

        // get items from the BSP tree and use them. there was junk in the tree
        // the second time this happened.
        foreach (QGraphicsItem *item, scene.items(QRectF(0, 0, 1000, 1000)))
            item->moveBy(0, 0);
    }
}

void tst_QGraphicsScene::task139782_containsItemBoundingRect()
{
    // The item in question has a scene bounding rect of (10, 10, 50, 50)
    QGraphicsScene scene(0.0, 0.0, 200.0, 200.0);
    QGraphicsRectItem *item = new QGraphicsRectItem(0.0, 0.0, 50.0, 50.0, 0, &scene);
    item->setPos(10.0, 10.0);

    // The (0, 0, 50, 50) scene rect should not include the item's bounding rect
    QVERIFY(!scene.items(QRectF(0.0, 0.0, 50.0, 50.0), Qt::ContainsItemBoundingRect).contains(item));

    // The (9, 9, 500, 500) scene rect _should_ include the item's bounding rect
    QVERIFY(scene.items(QRectF(9.0, 9.0, 500.0, 500.0), Qt::ContainsItemBoundingRect).contains(item));

    // The (25, 25, 5, 5) scene rect should not include the item's bounding rect
    QVERIFY(!scene.items(QRectF(25.0, 25.0, 5.0, 5.0), Qt::ContainsItemBoundingRect).contains(item));
}

QTEST_MAIN(tst_QGraphicsScene)
#include "tst_qgraphicsscene.moc"
#endif
