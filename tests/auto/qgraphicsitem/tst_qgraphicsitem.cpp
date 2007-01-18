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

#include <private/qtextcontrol_p.h>
#include <QAbstractTextDocumentLayout>
#include <QBitmap>
#include <QCursor>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QScrollBar>

//TESTED_CLASS=QGraphicsItem
//TESTED_FILES=gui/graphicsview/qgraphicsitem.cpp

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<QRectF>)
Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPointF)
Q_DECLARE_METATYPE(QRectF)

class EventTester : public QGraphicsItem
{
public:
    EventTester()
        : repaints(0)
    { }

    QRectF boundingRect() const
    { return QRectF(-10, -10, 20, 20); }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        hints = painter->renderHints();
        painter->drawRect(boundingRect());
        ++repaints;
    }

    bool sceneEvent(QEvent *event)
    {
        events << event->type();
        return QGraphicsItem::sceneEvent(event);
    }

    QList<QEvent::Type> events;
    QPainter::RenderHints hints;
    int repaints;
};

class tst_QGraphicsItem : public QObject
{
    Q_OBJECT

private slots:

    void construction();
    void constructionWithParent();
    void destruction();
    void scene();
    void parentItem();
    void setParentItem();
    void children();
    void flags();
    void toolTip();
    void visible();
    void enabled();
    void selected();
    void selected2();
    void selected_group();
    void selected_textItem();
    void selected_multi();
    void acceptedMouseButtons();
    void acceptsHoverEvents();
    void hasFocus();
    void pos();
    void scenePos();
    void matrix();
    void sceneMatrix();
    void setMatrix();
    void zValue();
    void shape();
    void contains();
    void collidesWith_item();
    void collidesWith_path_data();
    void collidesWith_path();
    void isObscuredBy();
    void isObscured();
    void mapFromToParent();
    void mapFromToScene();
    void mapFromToItem();
    void isAncestorOf();
    void data();
    void type();
    void graphicsitem_cast();
    void hoverEventsGenerateRepaints();
    void sceneBoundingRect();
    void childrenBoundingRect();
    void group();
    void setGroup();
    void nestedGroups();
    void warpChildrenIntoGroup();
    void handlesChildEvents();
    void handlesChildEvents2();
    void ensureVisible();
    void cursor();
    //void textControlGetterSetter();
    void defaultItemTest_QGraphicsPixmapItem();
    void itemChange();
    void sceneEventFilter();
    void prepareGeometryChange();
    void paint();
    void deleteItemInEventHandlers();
    void itemClipsToShape();
    void itemClipsChildrenToShape();

    // task specific tests below me
    void task141694_textItemEnsureVisible();
};

void tst_QGraphicsItem::construction()
{
    for (int i = 0; i < 7; ++i) {
        QGraphicsItem *item;
        switch (i) {
        case 0:
            item = new QGraphicsEllipseItem;
            QCOMPARE(int(item->type()), int(QGraphicsEllipseItem::Type));
            QCOMPARE(qgraphicsitem_cast<QGraphicsEllipseItem *>(item), (QGraphicsEllipseItem *)item);
            QCOMPARE(qgraphicsitem_cast<QGraphicsRectItem *>(item), (QGraphicsRectItem *)0);
            break;
        case 1:
            item = new QGraphicsLineItem;
            QCOMPARE(int(item->type()), int(QGraphicsLineItem::Type));
            QCOMPARE(qgraphicsitem_cast<QGraphicsLineItem *>(item), (QGraphicsLineItem *)item);
            QCOMPARE(qgraphicsitem_cast<QGraphicsRectItem *>(item), (QGraphicsRectItem *)0);
            break;
        case 2:
            item = new QGraphicsPathItem;
            QCOMPARE(int(item->type()), int(QGraphicsPathItem::Type));
            QCOMPARE(qgraphicsitem_cast<QGraphicsPathItem *>(item), (QGraphicsPathItem *)item);
            QCOMPARE(qgraphicsitem_cast<QGraphicsRectItem *>(item), (QGraphicsRectItem *)0);
            break;
        case 3:
            item = new QGraphicsPixmapItem;
            QCOMPARE(int(item->type()), int(QGraphicsPixmapItem::Type));
            QCOMPARE(qgraphicsitem_cast<QGraphicsPixmapItem *>(item), (QGraphicsPixmapItem *)item);
            QCOMPARE(qgraphicsitem_cast<QGraphicsRectItem *>(item), (QGraphicsRectItem *)0);
            break;
        case 4:
            item = new QGraphicsPolygonItem;
            QCOMPARE(int(item->type()), int(QGraphicsPolygonItem::Type));
            QCOMPARE(qgraphicsitem_cast<QGraphicsPolygonItem *>(item), (QGraphicsPolygonItem *)item);
            QCOMPARE(qgraphicsitem_cast<QGraphicsRectItem *>(item), (QGraphicsRectItem *)0);
            break;
        case 5:
            item = new QGraphicsRectItem;
            QCOMPARE(int(item->type()), int(QGraphicsRectItem::Type));
            QCOMPARE(qgraphicsitem_cast<QGraphicsRectItem *>(item), (QGraphicsRectItem *)item);
            QCOMPARE(qgraphicsitem_cast<QGraphicsLineItem *>(item), (QGraphicsLineItem *)0);
            break;
        case 6:
        default:
            item = new QGraphicsTextItem;
            QCOMPARE(int(item->type()), int(QGraphicsTextItem::Type));
            QCOMPARE(qgraphicsitem_cast<QGraphicsTextItem *>(item), (QGraphicsTextItem *)item);
            QCOMPARE(qgraphicsitem_cast<QGraphicsRectItem *>(item), (QGraphicsRectItem *)0);
            break;
        }

        QCOMPARE(item->scene(), (QGraphicsScene *)0);
        QCOMPARE(item->parentItem(), (QGraphicsItem *)0);
        QVERIFY(item->children().isEmpty());
        QCOMPARE(item->flags(), 0);
        QVERIFY(item->isVisible());
        QVERIFY(item->isEnabled());
        QVERIFY(!item->isSelected());
        QCOMPARE(item->acceptedMouseButtons(), Qt::MouseButtons(0x1f));
        if (item->type() == QGraphicsTextItem::Type)
            QVERIFY(item->acceptsHoverEvents());
        else
            QVERIFY(!item->acceptsHoverEvents());
        QVERIFY(!item->hasFocus());
        QCOMPARE(item->pos(), QPointF());
        QCOMPARE(item->matrix(), QMatrix());
        QCOMPARE(item->sceneMatrix(), QMatrix());
        QCOMPARE(item->zValue(), qreal(0));
        QCOMPARE(item->sceneBoundingRect(), QRectF());
        QCOMPARE(item->shape(), QPainterPath());
        QVERIFY(!item->contains(QPointF(0, 0)));
        QVERIFY(!item->collidesWithItem(0));
        QVERIFY(item->collidesWithItem(item));
        QVERIFY(!item->collidesWithPath(QPainterPath()));
        QVERIFY(!item->isAncestorOf(0));
        QVERIFY(!item->isAncestorOf(item));
        QCOMPARE(item->data(0), QVariant());
        delete item;
    }
}

class BoundingRectItem : public QGraphicsRectItem
{
public:
    BoundingRectItem(QGraphicsItem *parent = 0)
        : QGraphicsRectItem(0, 0, parent ? 200 : 100, parent ? 200 : 100,
                            parent)
    {}

    QRectF boundingRect() const
    {
        QRectF tmp = QGraphicsRectItem::boundingRect();
        foreach (QGraphicsItem *child, children())
            tmp |= child->boundingRect(); // <- might be pure virtual
        return tmp;
    }
};

void tst_QGraphicsItem::constructionWithParent()
{
    // This test causes a crash if item1 calls item2's pure virtuals before the
    // object has been constructed.
    QGraphicsItem *item0 = new BoundingRectItem;
    QGraphicsItem *item1 = new BoundingRectItem;
    QGraphicsScene scene;
    scene.addItem(item0);
    scene.addItem(item1);
    QGraphicsItem *item2 = new BoundingRectItem(item1);
    QCOMPARE(item1->children(), QList<QGraphicsItem *>() << item2);
    QCOMPARE(item1->boundingRect(), QRectF(0, 0, 200, 200));

    item2->setParentItem(item0);
    QCOMPARE(item0->children(), QList<QGraphicsItem *>() << item2);
    QCOMPARE(item0->boundingRect(), QRectF(0, 0, 200, 200));
}

static int itemDeleted = 0;
class Item : public QGraphicsRectItem
{
public:
    ~Item()
    { ++itemDeleted; }
};

void tst_QGraphicsItem::destruction()
{
    QCOMPARE(itemDeleted, 0);
    {
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        child->setParentItem(parent);
        QCOMPARE(child->parentItem(), parent);
        delete parent;
        QCOMPARE(itemDeleted, 1);
    }
    {
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        child->setParentItem(parent);
        QCOMPARE(parent->children().size(), 1);
        delete child;
        QCOMPARE(parent->children().size(), 0);
        delete parent;
        QCOMPARE(itemDeleted, 2);
    }
    {
        QGraphicsScene scene;
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        QCOMPARE(child->parentItem(), (QGraphicsItem *)0);
        child->setParentItem(parent);
        QCOMPARE(child->parentItem(), parent);
        scene.addItem(parent);
        QCOMPARE(child->parentItem(), parent);
        delete parent;
        QCOMPARE(itemDeleted, 3);
    }
    {
        QGraphicsScene scene;
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        child->setParentItem(parent);
        scene.addItem(parent);
        QCOMPARE(child->scene(), &scene);
        QCOMPARE(parent->children().size(), 1);
        delete child;
        QCOMPARE(parent->children().size(), 0);
        delete parent;
        QCOMPARE(itemDeleted, 4);
    }
    {
        QGraphicsScene scene;
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        child->setParentItem(parent);
        scene.addItem(parent);
        QCOMPARE(child->scene(), &scene);
        scene.removeItem(parent);
        QCOMPARE(child->scene(), (QGraphicsScene *)0);
        delete parent;
        QCOMPARE(itemDeleted, 5);
    }
    {
        QGraphicsScene scene;
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        child->setParentItem(parent);
        QCOMPARE(child->scene(), (QGraphicsScene *)0);
        QCOMPARE(parent->scene(), (QGraphicsScene *)0);
        scene.addItem(parent);
        QCOMPARE(child->scene(), &scene);
        scene.removeItem(child);
        QCOMPARE(child->scene(), (QGraphicsScene *)0);
        QCOMPARE(parent->scene(), &scene);
        QCOMPARE(child->parentItem(), (QGraphicsItem *)0);
        QVERIFY(parent->children().isEmpty());
        delete parent;
        QCOMPARE(itemDeleted, 5);
        delete child;
        QCOMPARE(itemDeleted, 6);
    }
    {
        QGraphicsScene scene;
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        child->setParentItem(parent);
        scene.addItem(parent);
        scene.removeItem(child);
        scene.removeItem(parent);
        delete child;
        delete parent;
        QCOMPARE(itemDeleted, 7);
    }
    {
        QGraphicsScene scene;
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        child->setParentItem(parent);
        scene.addItem(parent);
        QGraphicsScene scene2;
        scene2.addItem(parent);
        delete parent;
        QCOMPARE(itemDeleted, 8);
    }
    {
        QGraphicsScene scene;
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        child->setParentItem(parent);
        scene.addItem(parent);
        QCOMPARE(child->scene(), &scene);
        QGraphicsScene scene2;
        scene2.addItem(parent);
        QCOMPARE(child->scene(), &scene2);
        scene.addItem(parent);
        QCOMPARE(child->scene(), &scene);
        scene2.addItem(parent);
        QCOMPARE(child->scene(), &scene2);
        delete parent;
        QCOMPARE(itemDeleted, 9);
    }
    {
        QGraphicsScene scene;
        QGraphicsItem *parent = new QGraphicsRectItem;
        Item *child = new Item;
        child->setParentItem(parent);
        scene.addItem(parent);
        QCOMPARE(child->scene(), &scene);
        QGraphicsScene scene2;
        scene2.addItem(child);
        QCOMPARE(child->scene(), &scene2);
        delete parent;
        QCOMPARE(itemDeleted, 9);
        delete child;
        QCOMPARE(itemDeleted, 10);
    }
    {
        QGraphicsScene scene;
        QGraphicsItem *root = new QGraphicsRectItem;
        QGraphicsItem *parent = root;
        QGraphicsItem *middleItem = 0;
        for (int i = 0; i < 99; ++i) {
            Item *child = new Item;
            child->setParentItem(parent);
            parent = child;
            if (i == 50)
                middleItem = parent;
        }
        scene.addItem(root);

        QCOMPARE(scene.items().size(), 100);

        QGraphicsScene scene2;
        scene2.addItem(middleItem);

        delete middleItem;
        QCOMPARE(itemDeleted, 59);
    }
    QCOMPARE(itemDeleted, 109);
}

void tst_QGraphicsItem::scene()
{
    QGraphicsRectItem *item = new QGraphicsRectItem;
    QCOMPARE(item->scene(), (QGraphicsScene *)0);

    QGraphicsScene scene;
    scene.addItem(item);
    QCOMPARE(item->scene(), (QGraphicsScene *)&scene);

    QGraphicsScene scene2;
    scene2.addItem(item);
    QCOMPARE(item->scene(), (QGraphicsScene *)&scene2);

    scene2.removeItem(item);
    QCOMPARE(item->scene(), (QGraphicsScene *)0);

    delete item;
}

void tst_QGraphicsItem::parentItem()
{
    QGraphicsRectItem item;
    QCOMPARE(item.parentItem(), (QGraphicsItem *)0);

    QGraphicsRectItem *item2 = new QGraphicsRectItem(QRectF(), &item);
    QCOMPARE(item2->parentItem(), (QGraphicsItem *)&item);
    item2->setParentItem(&item);
    QCOMPARE(item2->parentItem(), (QGraphicsItem *)&item);
    item2->setParentItem(0);
    QCOMPARE(item2->parentItem(), (QGraphicsItem *)0);

    delete item2;
}

void tst_QGraphicsItem::setParentItem()
{
    QGraphicsScene scene;
    QGraphicsItem *item = scene.addRect(QRectF(0, 0, 10, 10));
    QCOMPARE(item->scene(), &scene);

    QGraphicsRectItem *child = new QGraphicsRectItem;
    QCOMPARE(child->scene(), (QGraphicsScene *)0);

    // This implicitly adds the item to the parent's scene
    child->setParentItem(item);
    QCOMPARE(child->scene(), &scene);

    // This just makes it a toplevel
    child->setParentItem(0);
    QCOMPARE(child->scene(), &scene);

    // Add the child back to the parent, then remove the parent from the scene
    child->setParentItem(item);
    scene.removeItem(item);
    QCOMPARE(child->scene(), (QGraphicsScene *)0);
}

void tst_QGraphicsItem::children()
{
    QGraphicsRectItem item;
    QVERIFY(item.children().isEmpty());

    QGraphicsRectItem *item2 = new QGraphicsRectItem(QRectF(), &item);
    QCOMPARE(item.children().size(), 1);
    QCOMPARE(item.children().first(), (QGraphicsItem *)item2);
    QVERIFY(item2->children().isEmpty());

    delete item2;
    QVERIFY(item.children().isEmpty());
}

void tst_QGraphicsItem::flags()
{
    QGraphicsRectItem *item = new QGraphicsRectItem(QRectF(-10, -10, 20, 20));
    QCOMPARE(item->flags(), 0);

    QGraphicsScene scene;
    scene.addItem(item);

    {
        // Focus
        item->setFlag(QGraphicsItem::ItemIsFocusable, false);
        QVERIFY(!item->hasFocus());
        item->setFocus();
        QVERIFY(!item->hasFocus());

        item->setFlag(QGraphicsItem::ItemIsFocusable, true);
        QVERIFY(!item->hasFocus());
        item->setFocus();
        QVERIFY(item->hasFocus());
        QVERIFY(scene.hasFocus());

        item->setFlag(QGraphicsItem::ItemIsFocusable, false);
        QVERIFY(!item->hasFocus());
        QVERIFY(scene.hasFocus());
    }
    {
        // Selectable
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        QVERIFY(!item->isSelected());
        item->setSelected(true);
        QVERIFY(!item->isSelected());

        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        QVERIFY(!item->isSelected());
        item->setSelected(true);
        QVERIFY(item->isSelected());
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        QVERIFY(!item->isSelected());
    }
    {
        // Movable
        item->setFlag(QGraphicsItem::ItemIsMovable, false);
        QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMousePress);
        event.setScenePos(QPointF(0, 0));
        event.setButton(Qt::LeftButton);
        event.setButtons(Qt::LeftButton);
        QApplication::sendEvent(&scene, &event);
        QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)item);

        QGraphicsSceneMouseEvent event2(QEvent::GraphicsSceneMouseMove);
        event2.setScenePos(QPointF(10, 10));
        event2.setButton(Qt::LeftButton);
        event2.setButtons(Qt::LeftButton);
        QApplication::sendEvent(&scene, &event2);
        QCOMPARE(item->pos(), QPointF());

        QGraphicsSceneMouseEvent event3(QEvent::GraphicsSceneMouseRelease);
        event3.setScenePos(QPointF(10, 10));
        event3.setButtons(0);
        QApplication::sendEvent(&scene, &event3);
        QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);

        item->setFlag(QGraphicsItem::ItemIsMovable, true);
        QGraphicsSceneMouseEvent event4(QEvent::GraphicsSceneMousePress);
        event4.setScenePos(QPointF(0, 0));
        event4.setButton(Qt::LeftButton);
        event4.setButtons(Qt::LeftButton);
        QApplication::sendEvent(&scene, &event4);
        QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)item);
        QGraphicsSceneMouseEvent event5(QEvent::GraphicsSceneMouseMove);
        event5.setScenePos(QPointF(10, 10));
        event5.setButton(Qt::LeftButton);
        event5.setButtons(Qt::LeftButton);
        QApplication::sendEvent(&scene, &event5);
        QCOMPARE(item->pos(), QPointF(10, 10));
    }
}

void tst_QGraphicsItem::toolTip()
{
    QString toolTip = "Qt rocks!";

    QGraphicsRectItem *item = new QGraphicsRectItem(QRectF(0, 0, 100, 100));
    item->setPen(QPen(Qt::red, 1));
    item->setBrush(QBrush(Qt::blue));
    QVERIFY(item->toolTip().isEmpty());
    item->setToolTip(toolTip);
    QCOMPARE(item->toolTip(), toolTip);

    QGraphicsScene scene;
    scene.addItem(item);

    QGraphicsView view(&scene);
    view.setFixedSize(200, 200);
    view.show();
    QTest::qWait(250);
    {
        QHelpEvent helpEvent(QEvent::ToolTip, view.viewport()->rect().topLeft(),
                             view.viewport()->mapToGlobal(view.viewport()->rect().topLeft()));
        QApplication::sendEvent(view.viewport(), &helpEvent);
        QTest::qWait(250);

        bool foundView = false;
        bool foundTipLabel = false;
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            if (widget == &view)
                foundView = true;
            if (widget->inherits("QTipLabel"))
                foundTipLabel = true;
        }
        QVERIFY(foundView);
        QVERIFY(!foundTipLabel);
    }

    {
        QHelpEvent helpEvent(QEvent::ToolTip, view.viewport()->rect().center(),
                             view.viewport()->mapToGlobal(view.viewport()->rect().center()));
        QApplication::sendEvent(view.viewport(), &helpEvent);
        QTest::qWait(250);

        bool foundView = false;
        bool foundTipLabel = false;
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            if (widget == &view)
                foundView = true;
            if (widget->inherits("QTipLabel"))
                foundTipLabel = true;
        }
        QVERIFY(foundView);
        QVERIFY(foundTipLabel);
    }

    {
        QHelpEvent helpEvent(QEvent::ToolTip, view.viewport()->rect().topLeft(),
                             view.viewport()->mapToGlobal(view.viewport()->rect().topLeft()));
        QApplication::sendEvent(view.viewport(), &helpEvent);
        QTest::qWait(1000);

        bool foundView = false;
        bool foundTipLabel = false;
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            if (widget == &view)
                foundView = true;
            if (widget->inherits("QTipLabel") && widget->isVisible())
                foundTipLabel = true;
        }
        QVERIFY(foundView);
        QVERIFY(!foundTipLabel);
    }
}

void tst_QGraphicsItem::visible()
{
    QGraphicsItem *item = new QGraphicsRectItem(QRectF(-10, -10, 20, 20));
    QVERIFY(item->isVisible());
    item->setVisible(false);
    QVERIFY(!item->isVisible());
    item->setVisible(true);
    QVERIFY(item->isVisible());

    QGraphicsScene scene;
    scene.addItem(item);
    QVERIFY(item->isVisible());
    QCOMPARE(scene.itemAt(0, 0), item);
    item->setVisible(false);
    QCOMPARE(scene.itemAt(0, 0), (QGraphicsItem *)0);
    item->setVisible(true);
    QCOMPARE(scene.itemAt(0, 0), item);

    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMousePress);
    event.setButton(Qt::LeftButton);
    event.setScenePos(QPointF(0, 0));
    QApplication::sendEvent(&scene, &event);
    QCOMPARE(scene.mouseGrabberItem(), item);
    item->setVisible(false);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);
    item->setVisible(true);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);

    item->setFlag(QGraphicsItem::ItemIsFocusable);
    item->setFocus();
    QVERIFY(item->hasFocus());
    item->setVisible(false);
    QVERIFY(!item->hasFocus());
    item->setVisible(true);
    QVERIFY(!item->hasFocus());
}

void tst_QGraphicsItem::enabled()
{
    QGraphicsRectItem *item = new QGraphicsRectItem(QRectF(-10, -10, 20, 20));
    QVERIFY(item->isEnabled());
    item->setEnabled(false);
    QVERIFY(!item->isEnabled());
    item->setEnabled(true);
    QVERIFY(item->isEnabled());
    item->setEnabled(false);
    item->setFlag(QGraphicsItem::ItemIsFocusable);
    QGraphicsScene scene;
    scene.addItem(item);
    item->setFocus();
    QVERIFY(!item->hasFocus());
    item->setEnabled(true);
    item->setFocus();
    QVERIFY(item->hasFocus());
    item->setEnabled(false);
    QVERIFY(!item->hasFocus());

    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMousePress);
    event.setButton(Qt::LeftButton);
    event.setScenePos(QPointF(0, 0));
    QApplication::sendEvent(&scene, &event);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);
    item->setEnabled(true);
    QApplication::sendEvent(&scene, &event);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)item);
    item->setEnabled(false);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);
}

class SelectChangeItem : public QGraphicsRectItem
{
public:
    SelectChangeItem() : QGraphicsRectItem(-50, -50, 100, 100) {}
    QList<bool> values;
    
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value)
    {
        if (change == ItemSelectedChange)
            values << value.toBool();
        return QGraphicsRectItem::itemChange(change, value);
    }
};

void tst_QGraphicsItem::selected()
{
    SelectChangeItem *item = new SelectChangeItem;
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    QVERIFY(!item->isSelected());
    QVERIFY(item->values.isEmpty());
    item->setSelected(true);
    QCOMPARE(item->values.size(), 1);
    QCOMPARE(item->values.last(), true);
    QVERIFY(item->isSelected());
    item->setSelected(false);
    QCOMPARE(item->values.size(), 2);
    QCOMPARE(item->values.last(), false);
    QVERIFY(!item->isSelected());
    item->setSelected(true);
    QCOMPARE(item->values.size(), 3);
    item->setEnabled(false);
    QCOMPARE(item->values.size(), 4);
    QCOMPARE(item->values.last(), false);
    QVERIFY(!item->isSelected());
    item->setEnabled(true);
    QCOMPARE(item->values.size(), 4);
    item->setSelected(true);
    QCOMPARE(item->values.size(), 5);
    QCOMPARE(item->values.last(), true);
    QVERIFY(item->isSelected());
    item->setVisible(false);
    QCOMPARE(item->values.size(), 6);
    QCOMPARE(item->values.last(), false);
    QVERIFY(!item->isSelected());
    item->setVisible(true);
    QCOMPARE(item->values.size(), 6);
    item->setSelected(true);
    QCOMPARE(item->values.size(), 7);
    QCOMPARE(item->values.last(), true);
    QVERIFY(item->isSelected());

    QGraphicsScene scene(-100, -100, 200, 200);
    scene.addItem(item);
    QCOMPARE(scene.selectedItems(), QList<QGraphicsItem *>() << item);
    item->setSelected(false);
    QVERIFY(scene.selectedItems().isEmpty());
    item->setSelected(true);
    QCOMPARE(scene.selectedItems(), QList<QGraphicsItem *>() << item);
    item->setSelected(false);
    QVERIFY(scene.selectedItems().isEmpty());

    // Interactive selection
    QGraphicsView view(&scene);
    view.setFixedSize(100, 100);
    view.show();

    qApp->processEvents();
    qApp->processEvents();

    scene.clearSelection();
    QCOMPARE(item->values.size(), 10);
    QCOMPARE(item->values.last(), false);
    QVERIFY(!item->isSelected());

    // Click inside and check that it's selected
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item->scenePos()));
    QCOMPARE(item->values.size(), 11);
    QCOMPARE(item->values.last(), true);
    QVERIFY(item->isSelected());

    // Click outside and check that it's not selected
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item->scenePos() + QPointF(item->boundingRect().width(), item->boundingRect().height())));

    QCOMPARE(item->values.size(), 12);
    QCOMPARE(item->values.last(), false);
    QVERIFY(!item->isSelected());

    SelectChangeItem *item2 = new SelectChangeItem;
    item2->setFlag(QGraphicsItem::ItemIsSelectable);
    item2->setPos(100, 0);
    scene.addItem(item2);

    // Click inside and check that it's selected
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item->scenePos()));
    QCOMPARE(item->values.size(), 13);
    QCOMPARE(item->values.last(), true);
    QVERIFY(item->isSelected());

    // Click inside item2 and check that it's selected, and item is not
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item2->scenePos()));
    QCOMPARE(item->values.size(), 14);
    QCOMPARE(item->values.last(), false);
    QVERIFY(!item->isSelected());
    QCOMPARE(item2->values.size(), 1);
    QCOMPARE(item2->values.last(), true);
    QVERIFY(item2->isSelected());
}

void tst_QGraphicsItem::selected2()
{
    // Selecting an item, then moving another previously caused a crash.
    QGraphicsScene scene;
    QGraphicsItem *line1 = scene.addRect(QRectF(0, 0, 100, 100));
    line1->setPos(-105, 0);
    line1->setFlag(QGraphicsItem::ItemIsSelectable);

    QGraphicsItem *line2 = scene.addRect(QRectF(0, 0, 100, 100));
    line2->setFlag(QGraphicsItem::ItemIsMovable);

    line1->setSelected(true);

    {
        QGraphicsSceneMouseEvent mousePress(QEvent::GraphicsSceneMousePress);
        mousePress.setScenePos(QPointF(50, 50));
        mousePress.setButton(Qt::LeftButton);
        QApplication::sendEvent(&scene, &mousePress);
        QVERIFY(mousePress.isAccepted());
    }
    {
        QGraphicsSceneMouseEvent mouseMove(QEvent::GraphicsSceneMouseMove);
        mouseMove.setScenePos(QPointF(60, 60));
        mouseMove.setButton(Qt::LeftButton);
        mouseMove.setButtons(Qt::LeftButton);
        QApplication::sendEvent(&scene, &mouseMove);
        QVERIFY(mouseMove.isAccepted());
    }
}

void tst_QGraphicsItem::selected_group()
{
    QGraphicsScene scene;
    QGraphicsItem *item1 = scene.addRect(QRectF());
    QGraphicsItem *item2 = scene.addRect(QRectF());
    item1->setFlag(QGraphicsItem::ItemIsSelectable);
    item2->setFlag(QGraphicsItem::ItemIsSelectable);
    scene.addRect(QRectF())->setParentItem(item1);
    QGraphicsItem *leaf = scene.addRect(QRectF());
    leaf->setFlag(QGraphicsItem::ItemIsSelectable);
    leaf->setParentItem(item2);

    QGraphicsItemGroup *group = scene.createItemGroup(QList<QGraphicsItem *>() << item1 << item2);
    QCOMPARE(group->scene(), &scene);
    group->setFlag(QGraphicsItem::ItemIsSelectable);
    foreach (QGraphicsItem *item, scene.items()) {
        if (item == group)
            QVERIFY(!item->group());
        else
            QCOMPARE(item->group(), group);
    }

    QVERIFY(group->handlesChildEvents());
    QVERIFY(!group->isSelected());
    group->setSelected(false);
    QVERIFY(!group->isSelected());
    group->setSelected(true);
    QVERIFY(group->isSelected());
    foreach (QGraphicsItem *item, scene.items())
        QVERIFY(item->isSelected());
    group->setSelected(false);
    QVERIFY(!group->isSelected());
    foreach (QGraphicsItem *item, scene.items())
        QVERIFY(!item->isSelected());   
    leaf->setSelected(true);
    foreach (QGraphicsItem *item, scene.items())
        QVERIFY(item->isSelected());
    leaf->setSelected(false);
    foreach (QGraphicsItem *item, scene.items())
        QVERIFY(!item->isSelected());

    leaf->setSelected(true);
    QGraphicsScene scene2;
    scene2.addItem(item1);
    QVERIFY(!item1->isSelected());
    QVERIFY(item2->isSelected());
}

void tst_QGraphicsItem::selected_textItem()
{
    QGraphicsScene scene;
    QGraphicsTextItem *text = scene.addText(QLatin1String("Text"));
    text->setFlag(QGraphicsItem::ItemIsSelectable);

    QGraphicsView view(&scene);
    view.show();
    QTest::qWait(1000);

    QVERIFY(!text->isSelected());
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0,
                      view.mapFromScene(text->mapToScene(0, 0)));
    QVERIFY(text->isSelected());

    text->setSelected(false);
    text->setTextInteractionFlags(Qt::TextEditorInteraction);
    
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0,
                      view.mapFromScene(text->mapToScene(0, 0)));
    QVERIFY(text->isSelected());
}

void tst_QGraphicsItem::selected_multi()
{
    // Test multiselection behavior
    QGraphicsScene scene;

    // Create two disjoint items
    QGraphicsItem *item1 = scene.addRect(QRectF(-10, -10, 20, 20));
    QGraphicsItem *item2 = scene.addRect(QRectF(-10, -10, 20, 20));
    item1->setPos(-15, 0);    
    item2->setPos(15, 20);    

    // Make both items selectable
    item1->setFlag(QGraphicsItem::ItemIsSelectable);
    item2->setFlag(QGraphicsItem::ItemIsSelectable);

    // Create and show a view
    QGraphicsView view(&scene);
    view.show();
    view.fitInView(scene.sceneRect());
    qApp->processEvents();

    QVERIFY(!item1->isSelected());
    QVERIFY(!item2->isSelected());

    // Start clicking
    QTest::qWait(200);

    // Click on item1
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(item1->isSelected());
    QVERIFY(!item2->isSelected());

    // Click on item2
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item2->scenePos()));
    QTest::qWait(200);
    QVERIFY(item2->isSelected());
    QVERIFY(!item1->isSelected());

    // Ctrl-click on item1
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(item2->isSelected());
    QVERIFY(item1->isSelected());

    // Ctrl-click on item1 again
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(item2->isSelected());
    QVERIFY(!item1->isSelected());

    // Ctrl-click on item2
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, view.mapFromScene(item2->scenePos()));
    QTest::qWait(200);
    QVERIFY(!item2->isSelected());
    QVERIFY(!item1->isSelected());

    // Click on item1
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(item1->isSelected());
    QVERIFY(!item2->isSelected());

    // Click on scene
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(0, 0));
    QTest::qWait(200);
    QVERIFY(!item1->isSelected());
    QVERIFY(!item2->isSelected());

    // Click on item1
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(item1->isSelected());
    QVERIFY(!item2->isSelected());

    // Ctrl-click on scene
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, view.mapFromScene(0, 0));
    QTest::qWait(200);
    QVERIFY(!item1->isSelected());
    QVERIFY(!item2->isSelected());

    // Click on item1
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(item1->isSelected());
    QVERIFY(!item2->isSelected());

    // Press on item2
    QTest::mousePress(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item2->scenePos()));
    QTest::qWait(200);
    QVERIFY(!item1->isSelected());
    QVERIFY(item2->isSelected());
    
    // Release on item2
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item2->scenePos()));
    QTest::qWait(200);
    QVERIFY(!item1->isSelected());
    QVERIFY(item2->isSelected());

    // Click on item1
    QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(item1->isSelected());
    QVERIFY(!item2->isSelected());

    // Ctrl-click on item1
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::ControlModifier, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(!item1->isSelected());
    QVERIFY(!item2->isSelected());

    // Ctrl-press on item1
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::ControlModifier, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(!item1->isSelected());
    QVERIFY(!item2->isSelected());

    {
        // Ctrl-move on item1
        QMouseEvent event(QEvent::MouseMove, view.mapFromScene(item1->scenePos()) + QPoint(1, 0), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
        QApplication::sendEvent(view.viewport(), &event);
        QTest::qWait(200);
        QVERIFY(!item1->isSelected());
        QVERIFY(!item2->isSelected());
    }

    // Release on item1
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::ControlModifier, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(item1->isSelected());
    QVERIFY(!item2->isSelected());

    item1->setFlag(QGraphicsItem::ItemIsMovable);
    item1->setSelected(false);

    // Ctrl-press on item1
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::ControlModifier, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(!item1->isSelected());
    QVERIFY(!item2->isSelected());

    {
        // Ctrl-move on item1
        QMouseEvent event(QEvent::MouseMove, view.mapFromScene(item1->scenePos()) + QPoint(1, 0), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
        QApplication::sendEvent(view.viewport(), &event);
        QTest::qWait(200);
        QVERIFY(item1->isSelected());
        QVERIFY(!item2->isSelected());
    }

    // Release on item1
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::ControlModifier, view.mapFromScene(item1->scenePos()));
    QTest::qWait(200);
    QVERIFY(item1->isSelected());
    QVERIFY(!item2->isSelected());
}

void tst_QGraphicsItem::acceptedMouseButtons()
{
    QGraphicsScene scene;
    QGraphicsRectItem *item1 = scene.addRect(QRectF(-10, -10, 20, 20));
    QGraphicsRectItem *item2 = scene.addRect(QRectF(-10, -10, 20, 20));
    item2->setZValue(1);

    item1->setFlag(QGraphicsItem::ItemIsMovable);
    item2->setFlag(QGraphicsItem::ItemIsMovable);

    QCOMPARE(item1->acceptedMouseButtons(), Qt::MouseButtons(0x1f));
    QCOMPARE(item2->acceptedMouseButtons(), Qt::MouseButtons(0x1f));

    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMousePress);
    event.setButton(Qt::LeftButton);
    event.setScenePos(QPointF(0, 0));
    QApplication::sendEvent(&scene, &event);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)item2);
    item2->setAcceptedMouseButtons(0);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)0);
    QApplication::sendEvent(&scene, &event);
    QCOMPARE(scene.mouseGrabberItem(), (QGraphicsItem *)item1);
}

class HoverItem : public QGraphicsRectItem
{
public:
    HoverItem(const QRectF &rect)
        : QGraphicsRectItem(rect), hoverInCount(0),
          hoverMoveCount(0), hoverOutCount(0)
    { }

    int hoverInCount;
    int hoverMoveCount;
    int hoverOutCount;
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *)
    { ++hoverInCount; }

    void hoverMoveEvent(QGraphicsSceneHoverEvent *)
    { ++hoverMoveCount; }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    { ++hoverOutCount; }
};

void tst_QGraphicsItem::acceptsHoverEvents()
{
    QGraphicsScene scene;
    HoverItem *item1 = new HoverItem(QRectF(-10, -10, 20, 20));
    HoverItem *item2 = new HoverItem(QRectF(-5, -5, 10, 10));
    scene.addItem(item1);
    scene.addItem(item2);
    item2->setZValue(1);

    QVERIFY(!item1->acceptsHoverEvents());
    QVERIFY(!item2->acceptsHoverEvents());
    item1->setAcceptsHoverEvents(true);
    item2->setAcceptsHoverEvents(true);

    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMouseMove);
    event.setScenePos(QPointF(-100, -100));
    QApplication::sendEvent(&scene, &event);
    event.setScenePos(QPointF(-2.5, -2.5));
    QApplication::sendEvent(&scene, &event);

    QCOMPARE(item1->hoverInCount, 0);
    QCOMPARE(item2->hoverInCount, 1);

    item1->setAcceptsHoverEvents(false);
    item2->setAcceptsHoverEvents(false);

    event.setScenePos(QPointF(-100, -100));
    QApplication::sendEvent(&scene, &event);
    event.setScenePos(QPointF(-2.5, -2.5));
    QApplication::sendEvent(&scene, &event);

    QCOMPARE(item1->hoverInCount, 0);
    QCOMPARE(item2->hoverInCount, 1);

    item1->setAcceptsHoverEvents(true);
    item2->setAcceptsHoverEvents(false);

    event.setScenePos(QPointF(-100, -100));
    QApplication::sendEvent(&scene, &event);
    event.setScenePos(QPointF(-2.5, -2.5));
    QApplication::sendEvent(&scene, &event);

    QCOMPARE(item1->hoverInCount, 1);
    QCOMPARE(item2->hoverInCount, 1);
}

void tst_QGraphicsItem::hasFocus()
{
    QGraphicsLineItem *line = new QGraphicsLineItem;
    QVERIFY(!line->hasFocus());
    line->setFocus();
    QVERIFY(!line->hasFocus());

    QGraphicsScene scene;
    scene.addItem(line);

    line->setFocus();
    QVERIFY(!line->hasFocus());
    line->setFlag(QGraphicsItem::ItemIsFocusable);
    line->setFocus();
    QVERIFY(line->hasFocus());

    QGraphicsScene scene2;
    scene2.addItem(line);
    QVERIFY(!line->hasFocus());

    QCOMPARE(scene.focusItem(), (QGraphicsItem *)0);
    QCOMPARE(scene2.focusItem(), (QGraphicsItem *)0);

    line->setFocus();
    QVERIFY(line->hasFocus());
    line->clearFocus();
    QVERIFY(!line->hasFocus());

    QGraphicsLineItem *line2 = new QGraphicsLineItem;
    line2->setFlag(QGraphicsItem::ItemIsFocusable);
    scene2.addItem(line2);

    line2->setFocus();
    QVERIFY(!line->hasFocus());
    QVERIFY(line2->hasFocus());
    line->setFocus();
    QVERIFY(line->hasFocus());
    QVERIFY(!line2->hasFocus());
}

void tst_QGraphicsItem::pos()
{
    QGraphicsItem *child = new QGraphicsLineItem;
    QGraphicsItem *parent = new QGraphicsLineItem;

    QCOMPARE(child->pos(), QPointF());
    QCOMPARE(parent->pos(), QPointF());

    child->setParentItem(parent);
    child->setPos(10, 10);

    QCOMPARE(child->pos(), QPointF(10, 10));

    parent->setPos(10, 10);

    QCOMPARE(parent->pos(), QPointF(10, 10));
    QCOMPARE(child->pos(), QPointF(10, 10));

    delete child;
    delete parent;
}

void tst_QGraphicsItem::scenePos()
{
    QGraphicsItem *child = new QGraphicsLineItem;
    QGraphicsItem *parent = new QGraphicsLineItem;

    QCOMPARE(child->scenePos(), QPointF());
    QCOMPARE(parent->scenePos(), QPointF());

    child->setParentItem(parent);
    child->setPos(10, 10);

    QCOMPARE(child->scenePos(), QPointF(10, 10));

    parent->setPos(10, 10);

    QCOMPARE(parent->scenePos(), QPointF(10, 10));
    QCOMPARE(child->scenePos(), QPointF(20, 20));

    parent->setPos(20, 20);

    QCOMPARE(parent->scenePos(), QPointF(20, 20));
    QCOMPARE(child->scenePos(), QPointF(30, 30));

    delete child;
    delete parent;
}

void tst_QGraphicsItem::matrix()
{
    QGraphicsLineItem line;
    QCOMPARE(line.matrix(), QMatrix());
    line.setMatrix(QMatrix().rotate(90));
    QCOMPARE(line.matrix(), QMatrix().rotate(90));
    line.setMatrix(QMatrix().rotate(90));
    QCOMPARE(line.matrix(), QMatrix().rotate(90));
    line.setMatrix(QMatrix().rotate(90), true);
    QCOMPARE(line.matrix(), QMatrix().rotate(180));
    line.setMatrix(QMatrix().rotate(-90), true);
    QCOMPARE(line.matrix(), QMatrix().rotate(90));
    line.resetMatrix();
    QCOMPARE(line.matrix(), QMatrix());

    line.rotate(90);
    QCOMPARE(line.matrix(), QMatrix().rotate(90));
    line.rotate(90);
    QCOMPARE(line.matrix(), QMatrix().rotate(90).rotate(90));
    line.resetMatrix();

    line.scale(2, 4);
    QCOMPARE(line.matrix(), QMatrix().scale(2, 4));
    line.scale(2, 4);
    QCOMPARE(line.matrix(), QMatrix().scale(2, 4).scale(2, 4));
    line.resetMatrix();

    line.shear(2, 4);
    QCOMPARE(line.matrix(), QMatrix().shear(2, 4));
    line.shear(2, 4);
    QCOMPARE(line.matrix(), QMatrix().shear(2, 4).shear(2, 4));
    line.resetMatrix();

    line.translate(10, 10);
    QCOMPARE(line.matrix(), QMatrix().translate(10, 10));
    line.translate(10, 10);
    QCOMPARE(line.matrix(), QMatrix().translate(10, 10).translate(10, 10));
    line.resetMatrix();
}

void tst_QGraphicsItem::sceneMatrix()
{
    QGraphicsLineItem *parent = new QGraphicsLineItem;
    QGraphicsLineItem *child = new QGraphicsLineItem(QLineF(), parent);

    QCOMPARE(parent->sceneMatrix(), QMatrix());
    QCOMPARE(child->sceneMatrix(), QMatrix());

    parent->translate(10, 10);
    QCOMPARE(parent->sceneMatrix(), QMatrix().translate(10, 10));
    QCOMPARE(child->sceneMatrix(), QMatrix().translate(10, 10));

    child->translate(10, 10);
    QCOMPARE(parent->sceneMatrix(), QMatrix().translate(10, 10));
    QCOMPARE(child->sceneMatrix(), QMatrix().translate(20, 20));

    parent->rotate(90);
    QCOMPARE(parent->sceneMatrix(), QMatrix().translate(10, 10).rotate(90));
    QCOMPARE(child->sceneMatrix(), QMatrix().translate(10, 10).rotate(90).translate(10, 10));

    delete child;
    delete parent;
}

void tst_QGraphicsItem::setMatrix()
{
    QGraphicsScene scene;
    qRegisterMetaType<QList<QRectF> >("QList<QRectF>");
    QSignalSpy spy(&scene, SIGNAL(changed(QList<QRectF>)));
    QRectF unrotatedRect(-12, -34, 56, 78);
    QGraphicsRectItem item(unrotatedRect, 0, &scene);
    scene.update(scene.sceneRect());
    QApplication::instance()->processEvents();

    QCOMPARE(spy.count(), 1);
    
    item.setMatrix(QMatrix().rotate(12.34));
    QRectF rotatedRect = scene.sceneRect();
    QVERIFY(unrotatedRect != rotatedRect);
    scene.update(scene.sceneRect());
    QApplication::instance()->processEvents();

    QCOMPARE(spy.count(), 2);

    item.setMatrix(QMatrix());
    scene.update(scene.sceneRect());
    QApplication::instance()->processEvents();

    QCOMPARE(spy.count(), 3);

    QList<QRectF> rlist = qVariantValue<QList<QRectF> >(spy.last().at(0));

    QCOMPARE(rlist.size(), 4);
    QCOMPARE(rlist.at(0), rotatedRect);   // From item.setMatrix() (clearing rotated rect)
    QCOMPARE(rlist.at(1), rotatedRect);   // ---''---              ---''---
    QCOMPARE(rlist.at(2), unrotatedRect); // ---''---              (painting unrotated rect)
    QCOMPARE(rlist.at(3), rotatedRect);   // From scene.update() (largest rect so far)
}

static QList<QGraphicsItem *> paintedItems;
class PainterItem : public QGraphicsItem
{
protected:
    QRectF boundingRect() const
    { return QRectF(-10, -10, 20, 20); }

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
    { paintedItems << this; }
};

void tst_QGraphicsItem::zValue()
{
    QGraphicsScene scene;

    QGraphicsItem *item1 = new PainterItem;
    QGraphicsItem *item2 = new PainterItem;
    QGraphicsItem *item3 = new PainterItem;
    QGraphicsItem *item4 = new PainterItem;
    scene.addItem(item1);
    scene.addItem(item2);
    scene.addItem(item3);
    scene.addItem(item4);
    item2->setZValue(-3);
    item4->setZValue(-2);
    item1->setZValue(-1);
    item3->setZValue(0);

    QGraphicsView view(&scene);
    view.show();
    QApplication::processEvents();

    QVERIFY(!paintedItems.isEmpty());
    QVERIFY((paintedItems.size() % 4) == 0);
    for (int i = 0; i < 3; ++i)
        QVERIFY(paintedItems.at(i)->zValue() < paintedItems.at(i + 1)->zValue());
}

void tst_QGraphicsItem::shape()
{
    QGraphicsLineItem line(QLineF(-10, -10, 20, 20));

    // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
    // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
    const qreal penWidthZero = qreal(0.00000001);

    QPainterPathStroker ps;
    ps.setWidth(penWidthZero);

    QPainterPath path(line.line().p1());
    path.lineTo(line.line().p2());
    QPainterPath p = ps.createStroke(path);
    p.addPath(path);
    QCOMPARE(line.shape(), p);

    QPen linePen;
    linePen.setWidthF(5.0);
    linePen.setCapStyle(Qt::RoundCap);
    line.setPen(linePen);

    ps.setCapStyle(line.pen().capStyle());
    ps.setWidth(line.pen().widthF());
    p = ps.createStroke(path);
    p.addPath(path);
    QCOMPARE(line.shape(), p);
    
    linePen.setCapStyle(Qt::FlatCap);
    line.setPen(linePen);
    ps.setCapStyle(line.pen().capStyle());
    p = ps.createStroke(path);
    p.addPath(path);
    QCOMPARE(line.shape(), p);

    linePen.setCapStyle(Qt::SquareCap);
    line.setPen(linePen);
    ps.setCapStyle(line.pen().capStyle());
    p = ps.createStroke(path);
    p.addPath(path);
    QCOMPARE(line.shape(), p);
 
    QGraphicsRectItem rect(QRectF(-10, -10, 20, 20));
    QPainterPathStroker ps1;
    ps1.setWidth(penWidthZero);
    path = QPainterPath();
    path.addRect(rect.rect());
    p = ps1.createStroke(path);
    p.addPath(path);
    QCOMPARE(rect.shape(), p);

    QGraphicsEllipseItem ellipse(QRectF(-10, -10, 20, 20));
    QPainterPathStroker ps2;
    ps2.setWidth(ellipse.pen().widthF() <= 0.0 ? penWidthZero : ellipse.pen().widthF());
    path = QPainterPath();
    path.addEllipse(ellipse.rect());
    p = ps2.createStroke(path);
    p.addPath(path);
    QCOMPARE(ellipse.shape(), p);

    QPainterPathStroker ps3;
    ps3.setWidth(penWidthZero);
    p = ps3.createStroke(path);
    p.addPath(path);
    QGraphicsPathItem pathItem(path);
    QCOMPARE(pathItem.shape(), p);

    QRegion region(QRect(0, 0, 300, 200));
    region = region.subtracted(QRect(50, 50, 200, 100));

    QImage image(300, 200, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter painter(&image);
    painter.setClipRegion(region);
    painter.fillRect(0, 0, 300, 200, Qt::green);
    painter.end();
    QPixmap pixmap = QPixmap::fromImage(image);

    QGraphicsPixmapItem pixmapItem(pixmap);
    path = QPainterPath();
    path.addRegion(region);

    QCOMPARE(pixmapItem.shape(), path);

    QPolygonF poly;
    poly << QPointF(0, 0) << QPointF(10, 0) << QPointF(0, 10);
    QGraphicsPolygonItem polygon(poly);
    path = QPainterPath();
    path.addPolygon(poly);

    QPainterPathStroker ps4;
    ps4.setWidth(penWidthZero);
    p = ps4.createStroke(path);
    p.addPath(path);   
    QCOMPARE(polygon.shape(), p);
}

void tst_QGraphicsItem::contains()
{
    // Rect
    QGraphicsRectItem rect(QRectF(-10, -10, 20, 20));
    QVERIFY(!rect.contains(QPointF(-11, -10)));
    QVERIFY(rect.contains(QPointF(-10, -10)));
    QVERIFY(!rect.contains(QPointF(-11, 0)));
    QVERIFY(rect.contains(QPointF(-10, 0)));
    QVERIFY(rect.contains(QPointF(0, -10)));
    QVERIFY(rect.contains(QPointF(0, 0)));
    QVERIFY(rect.contains(QPointF(9, 9)));

    // Ellipse
    QGraphicsEllipseItem ellipse(QRectF(-10, -10, 20, 20));
    QVERIFY(!ellipse.contains(QPointF(-10, -10)));
    QVERIFY(ellipse.contains(QPointF(-9, 0)));
    QVERIFY(ellipse.contains(QPointF(0, -9)));
    QVERIFY(ellipse.contains(QPointF(0, 0)));
    QVERIFY(!ellipse.contains(QPointF(9, 9)));

    // Line
    QGraphicsLineItem line(QLineF(-10, -10, 20, 20));
    QVERIFY(!line.contains(QPointF(-10, 0)));
    QVERIFY(!line.contains(QPointF(0, -10)));
    QVERIFY(!line.contains(QPointF(10, 0)));
    QVERIFY(!line.contains(QPointF(0, 10)));
    QVERIFY(line.contains(QPointF(0, 0)));
    QVERIFY(line.contains(QPointF(-9, -9)));
    QVERIFY(line.contains(QPointF(9, 9)));

    // Polygon
    QGraphicsPolygonItem polygon(QPolygonF()
                                 << QPointF(0, 0)
                                 << QPointF(10, 0)
                                 << QPointF(0, 10));
    QVERIFY(polygon.contains(QPointF(1, 1)));
    QVERIFY(polygon.contains(QPointF(4, 4)));
    QVERIFY(polygon.contains(QPointF(1, 4)));
    QVERIFY(polygon.contains(QPointF(4, 1)));
    QVERIFY(!polygon.contains(QPointF(8, 8)));
    QVERIFY(polygon.contains(QPointF(1, 8)));
    QVERIFY(polygon.contains(QPointF(8, 1)));
}

void tst_QGraphicsItem::collidesWith_item()
{
    // Rectangle
    QGraphicsRectItem rect(QRectF(-10, -10, 20, 20));
    QGraphicsRectItem rect2(QRectF(-10, -10, 20, 20));
    QVERIFY(rect.collidesWithItem(&rect2));
    QVERIFY(rect2.collidesWithItem(&rect));
    rect2.setPos(21, 21);
    QVERIFY(!rect.collidesWithItem(&rect2));
    QVERIFY(!rect2.collidesWithItem(&rect));
    rect2.setPos(-21, -21);
    QVERIFY(!rect.collidesWithItem(&rect2));
    QVERIFY(!rect2.collidesWithItem(&rect));
    rect2.setPos(-17, -17);
    QVERIFY(rect.collidesWithItem(&rect2));
    QVERIFY(rect2.collidesWithItem(&rect));

    QGraphicsEllipseItem ellipse(QRectF(-10, -10, 20, 20));
    QGraphicsEllipseItem ellipse2(QRectF(-10, -10, 20, 20));
    QVERIFY(ellipse.collidesWithItem(&ellipse2));
    QVERIFY(ellipse2.collidesWithItem(&ellipse));
    ellipse2.setPos(21, 21);
    QVERIFY(!ellipse.collidesWithItem(&ellipse2));
    QVERIFY(!ellipse2.collidesWithItem(&ellipse));
    ellipse2.setPos(-21, -21);
    QVERIFY(!ellipse.collidesWithItem(&ellipse2));
    QVERIFY(!ellipse2.collidesWithItem(&ellipse));

    ellipse2.setPos(-17, -17);
    QVERIFY(!ellipse.collidesWithItem(&ellipse2));
    QVERIFY(!ellipse2.collidesWithItem(&ellipse));

    {
        QGraphicsScene scene;
        QGraphicsRectItem rect(20, 20, 100, 100, 0, &scene);
        QGraphicsRectItem rect2(40, 40, 50, 50, 0, &scene);
        rect2.setZValue(1);
        QGraphicsLineItem line(0, 0, 200, 200, 0, &scene);
        line.setZValue(2);

        QCOMPARE(scene.items().size(), 3);

        QList<QGraphicsItem *> col1 = rect.collidingItems();
        QCOMPARE(col1.size(), 2);
        QCOMPARE(col1.first(), &line);
        QCOMPARE(col1.last(), &rect2);

        QList<QGraphicsItem *> col2 = rect2.collidingItems();
        QCOMPARE(col2.size(), 2);
        QCOMPARE(col2.first(), &line);
        QCOMPARE(col2.last(), &rect);

        QList<QGraphicsItem *> col3 = line.collidingItems();
        QCOMPARE(col3.size(), 2);
        QCOMPARE(col3.first(), &rect2);
        QCOMPARE(col3.last(), &rect);
    }
}

void tst_QGraphicsItem::collidesWith_path_data()
{
    QTest::addColumn<QPointF>("pos");
    QTest::addColumn<QMatrix>("matrix");
    QTest::addColumn<QPainterPath>("shape");
    QTest::addColumn<bool>("rectCollides");
    QTest::addColumn<bool>("ellipseCollides");

    QTest::newRow("nothing") << QPointF(0, 0) << QMatrix() << QPainterPath() << false << false;

    QPainterPath rect;
    rect.addRect(0, 0, 20, 20);

    QTest::newRow("rect1") << QPointF(0, 0) << QMatrix() << rect << true << true;
    QTest::newRow("rect2") << QPointF(0, 0) << QMatrix().translate(21, 21) << rect << false << false;
    QTest::newRow("rect3") << QPointF(21, 21) << QMatrix() << rect << false << false;
}

void tst_QGraphicsItem::collidesWith_path()
{
    QFETCH(QPointF, pos);
    QFETCH(QMatrix, matrix);
    QFETCH(QPainterPath, shape);
    QFETCH(bool, rectCollides);
    QFETCH(bool, ellipseCollides);

    QGraphicsRectItem rect(QRectF(0, 0, 20, 20));
    QGraphicsEllipseItem ellipse(QRectF(0, 0, 20, 20));

    rect.setPos(pos);
    rect.setMatrix(matrix);

    ellipse.setPos(pos);
    ellipse.setMatrix(matrix);

    QPainterPath mappedShape = rect.sceneMatrix().inverted().map(shape);

    if (rectCollides)
        QVERIFY(rect.collidesWithPath(mappedShape));
    else
        QVERIFY(!rect.collidesWithPath(mappedShape));

    if (ellipseCollides)
        QVERIFY(ellipse.collidesWithPath(mappedShape));
    else
        QVERIFY(!ellipse.collidesWithPath(mappedShape));
}

class MyItem : public QGraphicsEllipseItem
{
public:
    bool isObscuredBy(const QGraphicsItem *item) const
    {
        const MyItem *myItem = qgraphicsitem_cast<const MyItem *>(item);
        if (myItem) {
            if (item->zValue() > zValue()) {
                QRectF r = rect();
                QPointF topMid = (r.topRight()+r.topLeft())/2;
                QPointF botMid = (r.bottomRight()+r.bottomLeft())/2; 
                QPointF leftMid = (r.topLeft()+r.bottomLeft())/2;
                QPointF rightMid = (r.topRight()+r.bottomRight())/2;
                
                QPainterPath mappedShape = item->mapToItem(this, item->opaqueArea());
                
                if (mappedShape.contains(topMid) &&
                    mappedShape.contains(botMid) &&
                    mappedShape.contains(leftMid) &&
                    mappedShape.contains(rightMid))
                    return true;
                else
                    return false;
            }
            else return false;
        }
        else
            return QGraphicsItem::isObscuredBy(item);
    }
        
    QPainterPath opaqueArea() const
    {
        return shape();
    }

    enum {
        Type = UserType+1
    };
    int type() const { return Type; }
};

void tst_QGraphicsItem::isObscuredBy()
{
    QGraphicsScene scene;
    
    MyItem myitem1, myitem2;

    myitem1.setRect(QRectF(50, 50, 40, 200));
    myitem1.rotate(67);

    myitem2.setRect(QRectF(25, 25, 20, 20));
    myitem2.setZValue(-1.0);
    scene.addItem(&myitem1);
    scene.addItem(&myitem2);
    
    QVERIFY(!myitem2.isObscuredBy(&myitem1));
    QVERIFY(!myitem1.isObscuredBy(&myitem2));

    myitem2.setRect(QRectF(-50, 85, 20, 20));
    QVERIFY(myitem2.isObscuredBy(&myitem1));
    QVERIFY(!myitem1.isObscuredBy(&myitem2));

    myitem2.setRect(QRectF(-30, 70, 20, 20));
    QVERIFY(!myitem2.isObscuredBy(&myitem1));
    QVERIFY(!myitem1.isObscuredBy(&myitem2));

    QGraphicsRectItem rect1, rect2;

    rect1.setRect(QRectF(-40, -40, 50, 50));
    rect1.setBrush(QBrush(Qt::red));
    rect2.setRect(QRectF(-30, -20, 20, 20));    
    rect2.setZValue(-1.0);
    rect2.setBrush(QBrush(Qt::blue));
    
    QVERIFY(rect2.isObscuredBy(&rect1));
    QVERIFY(!rect1.isObscuredBy(&rect2));

    rect2.setPos(QPointF(-20, -25));

    QVERIFY(!rect2.isObscuredBy(&rect1));
    QVERIFY(!rect1.isObscuredBy(&rect2));

    rect2.setPos(QPointF(-100, -100));

    QVERIFY(!rect2.isObscuredBy(&rect1));
    QVERIFY(!rect1.isObscuredBy(&rect2));
}

class OpaqueItem : public QGraphicsRectItem
{
protected:
    QPainterPath opaqueArea() const
    {
        return shape();
    }
};

void tst_QGraphicsItem::isObscured()
{
    OpaqueItem *item1 = new OpaqueItem;
    item1->setRect(0, 0, 100, 100);
    item1->setZValue(0);

    OpaqueItem *item2 = new OpaqueItem;
    item2->setZValue(1);
    item2->setRect(0, 0, 100, 100);

    QGraphicsScene scene;
    scene.addItem(item1);
    scene.addItem(item2);
    
    QVERIFY(item1->isObscured());
    QVERIFY(item1->isObscuredBy(item2));
    QVERIFY(item1->isObscured(QRectF(0, 0, 50, 50)));
    QVERIFY(item1->isObscured(QRectF(50, 0, 50, 50)));
    QVERIFY(item1->isObscured(QRectF(50, 50, 50, 50)));
    QVERIFY(item1->isObscured(QRectF(0, 50, 50, 50)));
    QVERIFY(item1->isObscured(0, 0, 50, 50));
    QVERIFY(item1->isObscured(50, 0, 50, 50));
    QVERIFY(item1->isObscured(50, 50, 50, 50));
    QVERIFY(item1->isObscured(0, 50, 50, 50));
    QVERIFY(!item2->isObscured());
    QVERIFY(!item2->isObscuredBy(item1));
    QVERIFY(!item2->isObscured(QRectF(0, 0, 50, 50)));
    QVERIFY(!item2->isObscured(QRectF(50, 0, 50, 50)));
    QVERIFY(!item2->isObscured(QRectF(50, 50, 50, 50)));
    QVERIFY(!item2->isObscured(QRectF(0, 50, 50, 50)));
    QVERIFY(!item2->isObscured(0, 0, 50, 50));
    QVERIFY(!item2->isObscured(50, 0, 50, 50));
    QVERIFY(!item2->isObscured(50, 50, 50, 50));
    QVERIFY(!item2->isObscured(0, 50, 50, 50));

    item2->moveBy(50, 0);

    QVERIFY(!item1->isObscured());
    QVERIFY(!item1->isObscuredBy(item2));
    QVERIFY(!item1->isObscured(QRectF(0, 0, 50, 50)));
    QVERIFY(item1->isObscured(QRectF(50, 0, 50, 50)));
    QVERIFY(item1->isObscured(QRectF(50, 50, 50, 50)));
    QVERIFY(!item1->isObscured(QRectF(0, 50, 50, 50)));
    QVERIFY(!item1->isObscured(0, 0, 50, 50));
    QVERIFY(item1->isObscured(50, 0, 50, 50));
    QVERIFY(item1->isObscured(50, 50, 50, 50));
    QVERIFY(!item1->isObscured(0, 50, 50, 50));
    QVERIFY(!item2->isObscured());
    QVERIFY(!item2->isObscuredBy(item1));
    QVERIFY(!item2->isObscured(QRectF(0, 0, 50, 50)));
    QVERIFY(!item2->isObscured(QRectF(50, 0, 50, 50)));
    QVERIFY(!item2->isObscured(QRectF(50, 50, 50, 50)));
    QVERIFY(!item2->isObscured(QRectF(0, 50, 50, 50)));
    QVERIFY(!item2->isObscured(0, 0, 50, 50));
    QVERIFY(!item2->isObscured(50, 0, 50, 50));
    QVERIFY(!item2->isObscured(50, 50, 50, 50));
    QVERIFY(!item2->isObscured(0, 50, 50, 50));
}

void tst_QGraphicsItem::mapFromToParent()
{
    QPainterPath path1;
    path1.addRect(0, 0, 200, 200);

    QPainterPath path2;
    path2.addRect(0, 0, 100, 100);

    QPainterPath path3;
    path3.addRect(0, 0, 50, 50);

    QPainterPath path4;
    path4.addRect(0, 0, 25, 25);

    QGraphicsItem *item1 = new QGraphicsPathItem(path1);
    QGraphicsItem *item2 = new QGraphicsPathItem(path2, item1);
    QGraphicsItem *item3 = new QGraphicsPathItem(path3, item2);
    QGraphicsItem *item4 = new QGraphicsPathItem(path4, item3);

    item1->setPos(10, 10);
    item2->setPos(10, 10);
    item3->setPos(10, 10);
    item4->setPos(10, 10);

    for (int i = 0; i < 4; ++i) {
        QMatrix matrix;
        matrix.rotate(i * 90);
        matrix.translate(i * 100, -i * 100);
        matrix.scale(2, 4);
        item1->setMatrix(matrix);

        QCOMPARE(item1->mapToParent(QPointF(0, 0)), item1->pos() + matrix.map(QPointF(0, 0)));
        QCOMPARE(item2->mapToParent(QPointF(0, 0)), item2->pos());
        QCOMPARE(item3->mapToParent(QPointF(0, 0)), item3->pos());
        QCOMPARE(item4->mapToParent(QPointF(0, 0)), item4->pos());
        QCOMPARE(item1->mapToParent(QPointF(10, -10)), item1->pos() + matrix.map(QPointF(10, -10)));
        QCOMPARE(item2->mapToParent(QPointF(10, -10)), item2->pos() + QPointF(10, -10));
        QCOMPARE(item3->mapToParent(QPointF(10, -10)), item3->pos() + QPointF(10, -10));
        QCOMPARE(item4->mapToParent(QPointF(10, -10)), item4->pos() + QPointF(10, -10));
        QCOMPARE(item1->mapToParent(QPointF(-10, 10)), item1->pos() + matrix.map(QPointF(-10, 10)));
        QCOMPARE(item2->mapToParent(QPointF(-10, 10)), item2->pos() + QPointF(-10, 10));
        QCOMPARE(item3->mapToParent(QPointF(-10, 10)), item3->pos() + QPointF(-10, 10));
        QCOMPARE(item4->mapToParent(QPointF(-10, 10)), item4->pos() + QPointF(-10, 10));
        QCOMPARE(item1->mapFromParent(item1->pos()), matrix.inverted().map(QPointF(0, 0)));
        QCOMPARE(item2->mapFromParent(item2->pos()), QPointF(0, 0));
        QCOMPARE(item3->mapFromParent(item3->pos()), QPointF(0, 0));
        QCOMPARE(item4->mapFromParent(item4->pos()), QPointF(0, 0));
        QCOMPARE(item1->mapFromParent(item1->pos() + QPointF(10, -10)),
                 matrix.inverted().map(QPointF(10, -10)));
        QCOMPARE(item2->mapFromParent(item2->pos() + QPointF(10, -10)), QPointF(10, -10));
        QCOMPARE(item3->mapFromParent(item3->pos() + QPointF(10, -10)), QPointF(10, -10));
        QCOMPARE(item4->mapFromParent(item4->pos() + QPointF(10, -10)), QPointF(10, -10));
        QCOMPARE(item1->mapFromParent(item1->pos() + QPointF(-10, 10)),
                 matrix.inverted().map(QPointF(-10, 10)));
        QCOMPARE(item2->mapFromParent(item2->pos() + QPointF(-10, 10)), QPointF(-10, 10));
        QCOMPARE(item3->mapFromParent(item3->pos() + QPointF(-10, 10)), QPointF(-10, 10));
        QCOMPARE(item4->mapFromParent(item4->pos() + QPointF(-10, 10)), QPointF(-10, 10));
    }

    delete item1;
}

void tst_QGraphicsItem::mapFromToScene()
{
    QGraphicsItem *item1 = new QGraphicsPathItem(QPainterPath());
    QGraphicsItem *item2 = new QGraphicsPathItem(QPainterPath(), item1);
    QGraphicsItem *item3 = new QGraphicsPathItem(QPainterPath(), item2);
    QGraphicsItem *item4 = new QGraphicsPathItem(QPainterPath(), item3);

    item1->setPos(100, 100);
    item2->setPos(100, 100);
    item3->setPos(100, 100);
    item4->setPos(100, 100);
    QCOMPARE(item1->pos(), QPointF(100, 100));
    QCOMPARE(item2->pos(), QPointF(100, 100));
    QCOMPARE(item3->pos(), QPointF(100, 100));
    QCOMPARE(item4->pos(), QPointF(100, 100));
    QCOMPARE(item1->pos(), item1->mapToParent(0, 0));
    QCOMPARE(item2->pos(), item2->mapToParent(0, 0));
    QCOMPARE(item3->pos(), item3->mapToParent(0, 0));
    QCOMPARE(item4->pos(), item4->mapToParent(0, 0));
    QCOMPARE(item1->mapToParent(10, 10), QPointF(110, 110));
    QCOMPARE(item2->mapToParent(10, 10), QPointF(110, 110));
    QCOMPARE(item3->mapToParent(10, 10), QPointF(110, 110));
    QCOMPARE(item4->mapToParent(10, 10), QPointF(110, 110));
    QCOMPARE(item1->mapToScene(0, 0), QPointF(100, 100));
    QCOMPARE(item2->mapToScene(0, 0), QPointF(200, 200));
    QCOMPARE(item3->mapToScene(0, 0), QPointF(300, 300));
    QCOMPARE(item4->mapToScene(0, 0), QPointF(400, 400));
    QCOMPARE(item1->mapToScene(10, 0), QPointF(110, 100));
    QCOMPARE(item2->mapToScene(10, 0), QPointF(210, 200));
    QCOMPARE(item3->mapToScene(10, 0), QPointF(310, 300));
    QCOMPARE(item4->mapToScene(10, 0), QPointF(410, 400));
    QCOMPARE(item1->mapFromScene(100, 100), QPointF(0, 0));
    QCOMPARE(item2->mapFromScene(200, 200), QPointF(0, 0));
    QCOMPARE(item3->mapFromScene(300, 300), QPointF(0, 0));
    QCOMPARE(item4->mapFromScene(400, 400), QPointF(0, 0));
    QCOMPARE(item1->mapFromScene(110, 100), QPointF(10, 0));
    QCOMPARE(item2->mapFromScene(210, 200), QPointF(10, 0));
    QCOMPARE(item3->mapFromScene(310, 300), QPointF(10, 0));
    QCOMPARE(item4->mapFromScene(410, 400), QPointF(10, 0));

    // Rotate item1 90 degrees clockwise
    QMatrix matrix; matrix.rotate(90);
    item1->setMatrix(matrix);
    QCOMPARE(item1->pos(), item1->mapToParent(0, 0));
    QCOMPARE(item2->pos(), item2->mapToParent(0, 0));
    QCOMPARE(item3->pos(), item3->mapToParent(0, 0));
    QCOMPARE(item4->pos(), item4->mapToParent(0, 0));
    QCOMPARE(item1->mapToParent(10, 0), QPointF(100, 110));
    QCOMPARE(item2->mapToParent(10, 0), QPointF(110, 100));
    QCOMPARE(item3->mapToParent(10, 0), QPointF(110, 100));
    QCOMPARE(item4->mapToParent(10, 0), QPointF(110, 100));
    QCOMPARE(item1->mapToScene(0, 0), QPointF(100, 100));
    QCOMPARE(item2->mapToScene(0, 0), QPointF(0, 200));
    QCOMPARE(item3->mapToScene(0, 0), QPointF(-100, 300));
    QCOMPARE(item4->mapToScene(0, 0), QPointF(-200, 400));
    QCOMPARE(item1->mapToScene(10, 0), QPointF(100, 110));
    QCOMPARE(item2->mapToScene(10, 0), QPointF(0, 210));
    QCOMPARE(item3->mapToScene(10, 0), QPointF(-100, 310));
    QCOMPARE(item4->mapToScene(10, 0), QPointF(-200, 410));
    QCOMPARE(item1->mapFromScene(100, 100), QPointF(0, 0));
    QCOMPARE(item2->mapFromScene(0, 200), QPointF(0, 0));
    QCOMPARE(item3->mapFromScene(-100, 300), QPointF(0, 0));
    QCOMPARE(item4->mapFromScene(-200, 400), QPointF(0, 0));
    QCOMPARE(item1->mapFromScene(100, 110), QPointF(10, 0));
    QCOMPARE(item2->mapFromScene(0, 210), QPointF(10, 0));
    QCOMPARE(item3->mapFromScene(-100, 310), QPointF(10, 0));
    QCOMPARE(item4->mapFromScene(-200, 410), QPointF(10, 0));

    // Rotate item2 90 degrees clockwise
    item2->setMatrix(matrix);
    QCOMPARE(item1->pos(), item1->mapToParent(0, 0));
    QCOMPARE(item2->pos(), item2->mapToParent(0, 0));
    QCOMPARE(item3->pos(), item3->mapToParent(0, 0));
    QCOMPARE(item4->pos(), item4->mapToParent(0, 0));
    QCOMPARE(item1->mapToParent(10, 0), QPointF(100, 110));
    QCOMPARE(item2->mapToParent(10, 0), QPointF(100, 110));
    QCOMPARE(item3->mapToParent(10, 0), QPointF(110, 100));
    QCOMPARE(item4->mapToParent(10, 0), QPointF(110, 100));
    QCOMPARE(item1->mapToScene(0, 0), QPointF(100, 100));
    QCOMPARE(item2->mapToScene(0, 0), QPointF(0, 200));
    QCOMPARE(item3->mapToScene(0, 0), QPointF(-100, 100));
    QCOMPARE(item4->mapToScene(0, 0), QPointF(-200, 0));
    QCOMPARE(item1->mapToScene(10, 0), QPointF(100, 110));
    QCOMPARE(item2->mapToScene(10, 0), QPointF(-10, 200));
    QCOMPARE(item3->mapToScene(10, 0), QPointF(-110, 100));
    QCOMPARE(item4->mapToScene(10, 0), QPointF(-210, 0));
    QCOMPARE(item1->mapFromScene(100, 100), QPointF(0, 0));
    QCOMPARE(item2->mapFromScene(0, 200), QPointF(0, 0));
    QCOMPARE(item3->mapFromScene(-100, 100), QPointF(0, 0));
    QCOMPARE(item4->mapFromScene(-200, 0), QPointF(0, 0));
    QCOMPARE(item1->mapFromScene(100, 110), QPointF(10, 0));
    QCOMPARE(item2->mapFromScene(-10, 200), QPointF(10, 0));
    QCOMPARE(item3->mapFromScene(-110, 100), QPointF(10, 0));
    QCOMPARE(item4->mapFromScene(-210, 0), QPointF(10, 0));

    // Translate item3 50 points, then rotate 90 degrees counterclockwise
    QMatrix matrix2;
    matrix2.translate(50, 0);
    matrix2.rotate(-90);
    item3->setMatrix(matrix2);
    QCOMPARE(item1->pos(), item1->mapToParent(0, 0));
    QCOMPARE(item2->pos(), item2->mapToParent(0, 0));
    QCOMPARE(item3->pos(), item3->mapToParent(0, 0) - QPointF(50, 0));
    QCOMPARE(item4->pos(), item4->mapToParent(0, 0));
    QCOMPARE(item1->mapToParent(10, 0), QPointF(100, 110));
    QCOMPARE(item2->mapToParent(10, 0), QPointF(100, 110));
    QCOMPARE(item3->mapToParent(10, 0), QPointF(150, 90));
    QCOMPARE(item4->mapToParent(10, 0), QPointF(110, 100));
    QCOMPARE(item1->mapToScene(0, 0), QPointF(100, 100));
    QCOMPARE(item2->mapToScene(0, 0), QPointF(0, 200));
    QCOMPARE(item3->mapToScene(0, 0), QPointF(-150, 100));
    QCOMPARE(item4->mapToScene(0, 0), QPointF(-250, 200));
    QCOMPARE(item1->mapToScene(10, 0), QPointF(100, 110));
    QCOMPARE(item2->mapToScene(10, 0), QPointF(-10, 200));
    QCOMPARE(item3->mapToScene(10, 0), QPointF(-150, 110));
    QCOMPARE(item4->mapToScene(10, 0), QPointF(-250, 210));
    QCOMPARE(item1->mapFromScene(100, 100), QPointF(0, 0));
    QCOMPARE(item2->mapFromScene(0, 200), QPointF(0, 0));
    QCOMPARE(item3->mapFromScene(-150, 100), QPointF(0, 0));
    QCOMPARE(item4->mapFromScene(-250, 200), QPointF(0, 0));
    QCOMPARE(item1->mapFromScene(100, 110), QPointF(10, 0));
    QCOMPARE(item2->mapFromScene(-10, 200), QPointF(10, 0));
    QCOMPARE(item3->mapFromScene(-150, 110), QPointF(10, 0));
    QCOMPARE(item4->mapFromScene(-250, 210), QPointF(10, 0));

    delete item1;
}

void tst_QGraphicsItem::mapFromToItem()
{
    QGraphicsItem *item1 = new QGraphicsPathItem;
    QGraphicsItem *item2 = new QGraphicsPathItem;
    QGraphicsItem *item3 = new QGraphicsPathItem;
    QGraphicsItem *item4 = new QGraphicsPathItem;

    item1->setPos(-100, -100);
    item2->setPos(100, -100);
    item3->setPos(100, 100);
    item4->setPos(-100, 100);

    QCOMPARE(item1->mapFromItem(item2, 0, 0), QPointF(200, 0));
    QCOMPARE(item2->mapFromItem(item3, 0, 0), QPointF(0, 200));
    QCOMPARE(item3->mapFromItem(item4, 0, 0), QPointF(-200, 0));
    QCOMPARE(item4->mapFromItem(item1, 0, 0), QPointF(0, -200));
    QCOMPARE(item1->mapFromItem(item4, 0, 0), QPointF(0, 200));
    QCOMPARE(item2->mapFromItem(item1, 0, 0), QPointF(-200, 0));
    QCOMPARE(item3->mapFromItem(item2, 0, 0), QPointF(0, -200));
    QCOMPARE(item4->mapFromItem(item3, 0, 0), QPointF(200, 0));

    QMatrix matrix;
    matrix.translate(100, 100);
    item1->setMatrix(matrix);

    QCOMPARE(item1->mapFromItem(item2, 0, 0), QPointF(100, -100));
    QCOMPARE(item2->mapFromItem(item3, 0, 0), QPointF(0, 200));
    QCOMPARE(item3->mapFromItem(item4, 0, 0), QPointF(-200, 0));
    QCOMPARE(item4->mapFromItem(item1, 0, 0), QPointF(100, -100));
    QCOMPARE(item1->mapFromItem(item4, 0, 0), QPointF(-100, 100));
    QCOMPARE(item2->mapFromItem(item1, 0, 0), QPointF(-100, 100));
    QCOMPARE(item3->mapFromItem(item2, 0, 0), QPointF(0, -200));
    QCOMPARE(item4->mapFromItem(item3, 0, 0), QPointF(200, 0));

    matrix.rotate(90);
    item1->setMatrix(matrix);
    item2->setMatrix(matrix);
    item3->setMatrix(matrix);
    item4->setMatrix(matrix);

    QCOMPARE(item1->mapFromItem(item2, 0, 0), QPointF(0, -200));
    QCOMPARE(item2->mapFromItem(item3, 0, 0), QPointF(200, 0));
    QCOMPARE(item3->mapFromItem(item4, 0, 0), QPointF(0, 200));
    QCOMPARE(item4->mapFromItem(item1, 0, 0), QPointF(-200, 0));
    QCOMPARE(item1->mapFromItem(item4, 0, 0), QPointF(200, 0));
    QCOMPARE(item2->mapFromItem(item1, 0, 0), QPointF(0, 200));
    QCOMPARE(item3->mapFromItem(item2, 0, 0), QPointF(-200, 0));
    QCOMPARE(item4->mapFromItem(item3, 0, 0), QPointF(0, -200));
    QCOMPARE(item1->mapFromItem(item2, 10, -5), QPointF(10, -205));
    QCOMPARE(item2->mapFromItem(item3, 10, -5), QPointF(210, -5));
    QCOMPARE(item3->mapFromItem(item4, 10, -5), QPointF(10, 195));
    QCOMPARE(item4->mapFromItem(item1, 10, -5), QPointF(-190, -5));
    QCOMPARE(item1->mapFromItem(item4, 10, -5), QPointF(210, -5));
    QCOMPARE(item2->mapFromItem(item1, 10, -5), QPointF(10, 195));
    QCOMPARE(item3->mapFromItem(item2, 10, -5), QPointF(-190, -5));
    QCOMPARE(item4->mapFromItem(item3, 10, -5), QPointF(10, -205));

    QCOMPARE(item1->mapFromItem(0, 10, -5), item1->mapFromScene(10, -5));
    QCOMPARE(item2->mapFromItem(0, 10, -5), item2->mapFromScene(10, -5));
    QCOMPARE(item3->mapFromItem(0, 10, -5), item3->mapFromScene(10, -5));
    QCOMPARE(item4->mapFromItem(0, 10, -5), item4->mapFromScene(10, -5));
    QCOMPARE(item1->mapToItem(0, 10, -5), item1->mapToScene(10, -5));
    QCOMPARE(item2->mapToItem(0, 10, -5), item2->mapToScene(10, -5));
    QCOMPARE(item3->mapToItem(0, 10, -5), item3->mapToScene(10, -5));
    QCOMPARE(item4->mapToItem(0, 10, -5), item4->mapToScene(10, -5));

    delete item1;
    delete item2;
    delete item3;
    delete item4;
}

void tst_QGraphicsItem::isAncestorOf()
{
    QGraphicsItem *grandPa = new QGraphicsRectItem;
    QGraphicsItem *parent = new QGraphicsRectItem;
    QGraphicsItem *child = new QGraphicsRectItem;

    QVERIFY(!parent->isAncestorOf(0));
    QVERIFY(!child->isAncestorOf(0));
    QVERIFY(!parent->isAncestorOf(child));
    QVERIFY(!child->isAncestorOf(parent));
    QVERIFY(!parent->isAncestorOf(parent));

    child->setParentItem(parent);
    parent->setParentItem(grandPa);

    QVERIFY(parent->isAncestorOf(child));
    QVERIFY(grandPa->isAncestorOf(parent));
    QVERIFY(grandPa->isAncestorOf(child));
    QVERIFY(!child->isAncestorOf(parent));
    QVERIFY(!parent->isAncestorOf(grandPa));
    QVERIFY(!child->isAncestorOf(grandPa));
    QVERIFY(!child->isAncestorOf(child));
    QVERIFY(!parent->isAncestorOf(parent));
    QVERIFY(!grandPa->isAncestorOf(grandPa));

    parent->setParentItem(0);

    delete child;
    delete parent;
    delete grandPa;
}

void tst_QGraphicsItem::data()
{
    QGraphicsTextItem text;

    QCOMPARE(text.data(0), QVariant());
    text.setData(0, "TextItem");
    QCOMPARE(text.data(0), QVariant(QString("TextItem")));
    text.setData(0, QVariant());
    QCOMPARE(text.data(0), QVariant());
}

void tst_QGraphicsItem::type()
{
    QCOMPARE(int(QGraphicsItem::Type), 1);
    QCOMPARE(int(QGraphicsPathItem::Type), 2);
    QCOMPARE(int(QGraphicsRectItem::Type), 3);
    QCOMPARE(int(QGraphicsEllipseItem::Type), 4);
    QCOMPARE(int(QGraphicsPolygonItem::Type), 5);
    QCOMPARE(int(QGraphicsLineItem::Type), 6);
    QCOMPARE(int(QGraphicsPixmapItem::Type), 7);
    QCOMPARE(int(QGraphicsTextItem::Type), 8);

    QCOMPARE(QGraphicsPathItem().type(), 2);
    QCOMPARE(QGraphicsRectItem().type(), 3);
    QCOMPARE(QGraphicsEllipseItem().type(), 4);
    QCOMPARE(QGraphicsPolygonItem().type(), 5);
    QCOMPARE(QGraphicsLineItem().type(), 6);
    QCOMPARE(QGraphicsPixmapItem().type(), 7);
    QCOMPARE(QGraphicsTextItem().type(), 8);
}

void tst_QGraphicsItem::graphicsitem_cast()
{
    QGraphicsPathItem pathItem;
    const QGraphicsPathItem *pPathItem = &pathItem;
    QGraphicsRectItem rectItem;
    const QGraphicsRectItem *pRectItem = &rectItem;
    QGraphicsEllipseItem ellipseItem;
    const QGraphicsEllipseItem *pEllipseItem = &ellipseItem;
    QGraphicsPolygonItem polygonItem;
    const QGraphicsPolygonItem *pPolygonItem = &polygonItem;        
    QGraphicsLineItem lineItem;
    const QGraphicsLineItem *pLineItem = &lineItem;
    QGraphicsPixmapItem pixmapItem;
    const QGraphicsPixmapItem *pPixmapItem = &pixmapItem;
    QGraphicsTextItem textItem;
    const QGraphicsTextItem *pTextItem = &textItem;
    
    QVERIFY(qgraphicsitem_cast<QGraphicsPathItem *>(&pathItem));
    //QVERIFY(qgraphicsitem_cast<QAbstractGraphicsPathItem *>(&pathItem));
    QVERIFY(qgraphicsitem_cast<QGraphicsItem *>(&pathItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsItem *>(pPathItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsPathItem *>(pPathItem));

    QVERIFY(qgraphicsitem_cast<QGraphicsRectItem *>(&rectItem));
    QVERIFY(qgraphicsitem_cast<QGraphicsItem *>(&rectItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsItem *>(pRectItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsRectItem *>(pRectItem));

    QVERIFY(qgraphicsitem_cast<QGraphicsEllipseItem *>(&ellipseItem));
    QVERIFY(qgraphicsitem_cast<QGraphicsItem *>(&ellipseItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsItem *>(pEllipseItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsEllipseItem *>(pEllipseItem));

    QVERIFY(qgraphicsitem_cast<QGraphicsPolygonItem *>(&polygonItem));
    //QVERIFY(qgraphicsitem_cast<QAbstractGraphicsPathItem *>(&polygonItem));
    QVERIFY(qgraphicsitem_cast<QGraphicsItem *>(&polygonItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsItem *>(pPolygonItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsPolygonItem *>(pPolygonItem));

    QVERIFY(qgraphicsitem_cast<QGraphicsLineItem *>(&lineItem));
    QVERIFY(qgraphicsitem_cast<QGraphicsItem *>(&lineItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsItem *>(pLineItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsLineItem *>(pLineItem));

    QVERIFY(qgraphicsitem_cast<QGraphicsPixmapItem *>(&pixmapItem));
    QVERIFY(qgraphicsitem_cast<QGraphicsItem *>(&pixmapItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsItem *>(pPixmapItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsPixmapItem *>(pPixmapItem));

    QVERIFY(qgraphicsitem_cast<QGraphicsTextItem *>(&textItem));
    QVERIFY(qgraphicsitem_cast<QGraphicsItem *>(&textItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsItem *>(pTextItem));
    QVERIFY(qgraphicsitem_cast<const QGraphicsTextItem *>(pTextItem));

    // and some casts that _should_ fail:
    QVERIFY(!qgraphicsitem_cast<QGraphicsEllipseItem *>(&pathItem));
    QVERIFY(!qgraphicsitem_cast<const QGraphicsTextItem *>(pPolygonItem));

    // and this shouldn't crash
    QGraphicsItem *ptr = 0;
    QVERIFY(!qgraphicsitem_cast<QGraphicsTextItem *>(ptr));
    QVERIFY(!qgraphicsitem_cast<QGraphicsItem *>(ptr));
}

void tst_QGraphicsItem::hoverEventsGenerateRepaints()
{
    QGraphicsScene scene;
    EventTester *tester = new EventTester;
    scene.addItem(tester);
    tester->setAcceptsHoverEvents(true);

    QGraphicsView view(&scene);
    view.show();

    qApp->processEvents();
    qApp->processEvents();

    // Send a hover enter event
    QGraphicsSceneHoverEvent hoverEnterEvent(QEvent::GraphicsSceneHoverEnter);
    hoverEnterEvent.setScenePos(QPointF(0, 0));
    hoverEnterEvent.setPos(QPointF(0, 0));
    QApplication::sendEvent(&scene, &hoverEnterEvent);

    // Check that we get a repaint
    int npaints = tester->repaints;
    qApp->processEvents();
    qApp->processEvents();
    QCOMPARE(tester->events.size(), 1);
    QCOMPARE(tester->repaints, npaints + 1);
    QCOMPARE(tester->events.last(), QEvent::GraphicsSceneHoverEnter);

    // Send a hover move event
    QGraphicsSceneHoverEvent hoverMoveEvent(QEvent::GraphicsSceneHoverMove);
    hoverMoveEvent.setScenePos(QPointF(0, 0));
    hoverMoveEvent.setPos(QPointF(0, 0));
    QApplication::sendEvent(&scene, &hoverMoveEvent);

    // Check that we don't get a repaint
    qApp->processEvents();
    qApp->processEvents();

    QCOMPARE(tester->events.size(), 2);
    QCOMPARE(tester->repaints, npaints + 1);
    QCOMPARE(tester->events.last(), QEvent::GraphicsSceneHoverMove);

    // Send a hover leave event
    QGraphicsSceneHoverEvent hoverLeaveEvent(QEvent::GraphicsSceneHoverLeave);
    hoverLeaveEvent.setScenePos(QPointF(-100, -100));
    hoverLeaveEvent.setPos(QPointF(0, 0));
    QApplication::sendEvent(&scene, &hoverLeaveEvent);

    // Check that we get a repaint
    qApp->processEvents();
    qApp->processEvents();

    QCOMPARE(tester->events.size(), 3);
    QCOMPARE(tester->repaints, npaints + 2);
    QCOMPARE(tester->events.last(), QEvent::GraphicsSceneHoverLeave);
}

void tst_QGraphicsItem::sceneBoundingRect()
{
    QGraphicsScene scene;
    QGraphicsRectItem *item = scene.addRect(QRectF(0, 0, 100, 100), QPen(Qt::black, 0));
    item->setPos(100, 100);

    QCOMPARE(item->boundingRect(), QRectF(0, 0, 100, 100));
    QCOMPARE(item->sceneBoundingRect(), QRectF(100, 100, 100, 100));

    item->rotate(90);

    QCOMPARE(item->boundingRect(), QRectF(0, 0, 100, 100));
    QCOMPARE(item->sceneBoundingRect(), QRectF(0, 100, 100, 100));
}

void tst_QGraphicsItem::childrenBoundingRect()
{
    QGraphicsScene scene;
    QGraphicsRectItem *parent = scene.addRect(QRectF(0, 0, 100, 100), QPen(Qt::black, 0));
    QGraphicsRectItem *child = scene.addRect(QRectF(0, 0, 100, 100), QPen(Qt::black, 0));
    child->setParentItem(parent);
    parent->setPos(100, 100);
    child->setPos(100, 100);

    QCOMPARE(parent->boundingRect(), QRectF(0, 0, 100, 100));
    QCOMPARE(child->boundingRect(), QRectF(0, 0, 100, 100));
    QCOMPARE(child->childrenBoundingRect(), QRectF());
    QCOMPARE(parent->childrenBoundingRect(), QRectF(100, 100, 100, 100));

    QGraphicsRectItem *child2 = scene.addRect(QRectF(0, 0, 100, 100), QPen(Qt::black, 0));
    child2->setParentItem(parent);
    child2->setPos(-100, -100);
    QCOMPARE(parent->childrenBoundingRect(), QRectF(-100, -100, 300, 300));

    QGraphicsRectItem *childChild = scene.addRect(QRectF(0, 0, 100, 100), QPen(Qt::black, 0));
    childChild->setParentItem(child);
    childChild->setPos(500, 500);
    child->rotate(90);
    QCOMPARE(parent->childrenBoundingRect(), QRectF(-500, -100, 600, 800));
}

void tst_QGraphicsItem::group()
{
    QGraphicsScene scene;
    QGraphicsRectItem *parent = scene.addRect(QRectF(0, 0, 50, 50), QPen(Qt::black, 0), QBrush(Qt::green));
    QGraphicsRectItem *child = scene.addRect(QRectF(0, 0, 50, 50), QPen(Qt::black, 0), QBrush(Qt::blue));
    QGraphicsRectItem *parent2 = scene.addRect(QRectF(0, 0, 50, 50), QPen(Qt::black, 0), QBrush(Qt::red));
    parent2->setPos(-50, 50);
    child->rotate(45);
    child->setParentItem(parent);
    parent->setPos(25, 25);
    child->setPos(25, 25);

    QCOMPARE(parent->group(), (QGraphicsItemGroup *)0);
    QCOMPARE(parent2->group(), (QGraphicsItemGroup *)0);
    QCOMPARE(child->group(), (QGraphicsItemGroup *)0);
    
    QGraphicsView view(&scene);
    view.show();
    QTest::qWait(250);

    QGraphicsItemGroup *group = new QGraphicsItemGroup;
    group->setSelected(true);
    scene.addItem(group);

    QRectF parentSceneBoundingRect = parent->sceneBoundingRect();
    group->addToGroup(parent);
    QCOMPARE(parent->group(), group);
    QCOMPARE(parent->sceneBoundingRect(), parentSceneBoundingRect);
    
    QCOMPARE(parent->parentItem(), (QGraphicsItem *)group);
    QCOMPARE(group->children().size(), 1);
    QCOMPARE(scene.items().size(), 4);
    QCOMPARE(scene.items(group->sceneBoundingRect()).size(), 3);
    
    QTest::qWait(250);

    QRectF parent2SceneBoundingRect = parent2->sceneBoundingRect();
    group->addToGroup(parent2);
    QCOMPARE(parent2->group(), group);
    QCOMPARE(parent2->sceneBoundingRect(), parent2SceneBoundingRect);

    QCOMPARE(parent2->parentItem(), (QGraphicsItem *)group);
    QCOMPARE(group->children().size(), 2);
    QCOMPARE(scene.items().size(), 4);
    QCOMPARE(scene.items(group->sceneBoundingRect()).size(), 4);

    QTest::qWait(250);

    QList<QGraphicsItem *> newItems;
    for (int i = 0; i < 100; ++i) {
        QGraphicsItem *item = scene.addRect(QRectF(-25, -25, 50, 50), QPen(Qt::black, 0),
                                            QBrush(QColor(rand() % 255, rand() % 255,
                                                          rand() % 255, rand() % 255)));
        newItems << item;
        item->setPos(-1000 + rand() % 2000,
                     -1000 + rand() % 2000);
        item->rotate(rand() % 90);
    }

    view.fitInView(scene.itemsBoundingRect());

    int n = 0;
    foreach (QGraphicsItem *item, newItems) {
        group->addToGroup(item);
        QCOMPARE(item->group(), group);
        if ((n++ % 100) == 0)
            QTest::qWait(10);
    }
}

void tst_QGraphicsItem::setGroup()
{
    QGraphicsItemGroup group1;
    QGraphicsItemGroup group2;

    QGraphicsRectItem *rect = new QGraphicsRectItem;
    QCOMPARE(rect->group(), (QGraphicsItemGroup *)0);
    QCOMPARE(rect->parentItem(), (QGraphicsItem *)0);
    rect->setGroup(&group1);
    QCOMPARE(rect->group(), &group1);
    QCOMPARE(rect->parentItem(), (QGraphicsItem *)&group1);
    rect->setGroup(&group2);
    QCOMPARE(rect->group(), &group2);
    QCOMPARE(rect->parentItem(), (QGraphicsItem *)&group2);
    rect->setGroup(0);
    QCOMPARE(rect->group(), (QGraphicsItemGroup *)0);
    QCOMPARE(rect->parentItem(), (QGraphicsItem *)0);
}

void tst_QGraphicsItem::nestedGroups()
{
    QGraphicsItemGroup *group1 = new QGraphicsItemGroup;
    QGraphicsItemGroup *group2 = new QGraphicsItemGroup;

    QGraphicsRectItem *rect = new QGraphicsRectItem;
    QGraphicsRectItem *rect2 = new QGraphicsRectItem;
    rect2->setParentItem(rect);

    group1->addToGroup(rect);
    QCOMPARE(rect->group(), group1);
    QCOMPARE(rect2->group(), group1);

    group2->addToGroup(group1);
    QCOMPARE(rect->group(), group1);
    QCOMPARE(rect2->group(), group1);
    QCOMPARE(group1->group(), group2);
    QCOMPARE(group2->group(), (QGraphicsItemGroup *)0);

    QGraphicsScene scene;
    scene.addItem(group1);
    
    QCOMPARE(rect->group(), group1);
    QCOMPARE(rect2->group(), group1);
    QCOMPARE(group1->group(), (QGraphicsItemGroup *)0);
    QVERIFY(group2->children().isEmpty());

    delete group2;
}

void tst_QGraphicsItem::warpChildrenIntoGroup()
{
    QGraphicsScene scene;
    QGraphicsRectItem *parentRectItem = scene.addRect(QRectF(0, 0, 100, 100));
    QGraphicsRectItem *childRectItem = scene.addRect(QRectF(0, 0, 100, 100));
    parentRectItem->rotate(90);
    childRectItem->setPos(-50, -25);
    childRectItem->setParentItem(parentRectItem);

    QCOMPARE(childRectItem->mapToScene(50, 0), QPointF(25, 0));
    QCOMPARE(childRectItem->scenePos(), QPointF(25, -50));

    QGraphicsRectItem *parentOfGroup = scene.addRect(QRectF(0, 0, 100, 100));
    parentOfGroup->setPos(-200, -200);
    parentOfGroup->scale(2, 2);

    QGraphicsItemGroup *group = new QGraphicsItemGroup;
    group->setPos(50, 50);
    group->setParentItem(parentOfGroup);

    QCOMPARE(group->scenePos(), QPointF(-100, -100));

    group->addToGroup(childRectItem);

    QCOMPARE(childRectItem->mapToScene(50, 0), QPointF(25, 0));
    QCOMPARE(childRectItem->scenePos(), QPointF(25, -50));
}

class ChildEventTester : public QGraphicsRectItem
{
public:
    ChildEventTester(const QRectF &rect, QGraphicsItem *parent = 0)
        : QGraphicsRectItem(rect, parent), counter(0)
    { }

    int numMouseEvents() const
    { return counter; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *)
    { ++counter; }
    void mouseMoveEvent(QGraphicsSceneMouseEvent *)
    { ++counter; }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *)
    { ++counter; }

private:
    int counter;
};

void tst_QGraphicsItem::handlesChildEvents()
{
    ChildEventTester *item2_level2 = new ChildEventTester(QRectF(0, 0, 25, 25));
    ChildEventTester *item1_level1 = new ChildEventTester(QRectF(0, 0, 50, 50));
    ChildEventTester *item_level0 = new ChildEventTester(QRectF(0, 0, 100, 100));
    ChildEventTester *item1_level2 = new ChildEventTester(QRectF(0, 0, 25, 25));
    ChildEventTester *item2_level1 = new ChildEventTester(QRectF(0, 0, 50, 50));

    item_level0->setBrush(QBrush(Qt::blue));
    item1_level1->setBrush(QBrush(Qt::red));
    item2_level1->setBrush(QBrush(Qt::yellow));
    item1_level2->setBrush(QBrush(Qt::green));
    item2_level2->setBrush(QBrush(Qt::gray));
    item1_level1->setPos(50, 0);
    item2_level1->setPos(50, 50);
    item1_level2->setPos(25, 0);
    item2_level2->setPos(25, 25);
    item1_level1->setParentItem(item_level0);
    item2_level1->setParentItem(item_level0);
    item1_level2->setParentItem(item1_level1);
    item2_level2->setParentItem(item1_level1);

    QGraphicsScene scene;
    scene.addItem(item_level0);

    // Pull out the items, closest item first
    QList<QGraphicsItem *> items = scene.items(scene.itemsBoundingRect());
    QCOMPARE(items.at(4), (QGraphicsItem *)item_level0);
    if (item1_level1 < item2_level1) {
        QCOMPARE(items.at(3), (QGraphicsItem *)item1_level1);
        if (item1_level2 < item2_level2) {
            QCOMPARE(items.at(2), (QGraphicsItem *)item1_level2);
            QCOMPARE(items.at(1), (QGraphicsItem *)item2_level2);
        } else {
            QCOMPARE(items.at(2), (QGraphicsItem *)item2_level2);
            QCOMPARE(items.at(1), (QGraphicsItem *)item1_level2);
        }
        QCOMPARE(items.at(0), (QGraphicsItem *)item2_level1);
    } else {
        QCOMPARE(items.at(3), (QGraphicsItem *)item2_level1);
        QCOMPARE(items.at(2), (QGraphicsItem *)item1_level1);
        if (item1_level2 < item2_level2) {
            QCOMPARE(items.at(1), (QGraphicsItem *)item1_level2);
            QCOMPARE(items.at(0), (QGraphicsItem *)item2_level2);
        } else {
            QCOMPARE(items.at(1), (QGraphicsItem *)item2_level2);
            QCOMPARE(items.at(0), (QGraphicsItem *)item1_level2);
        }
    }

    QGraphicsView view(&scene);
    view.show();

    QTest::qWait(1000);

    QCOMPARE(item_level0->numMouseEvents(), 0);

    // Send events to the toplevel item
    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    QGraphicsSceneMouseEvent releaseEvent(QEvent::GraphicsSceneMouseRelease);

    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setScenePos(item_level0->mapToScene(5, 5));
    pressEvent.setScreenPos(view.mapFromScene(pressEvent.scenePos()));
    releaseEvent.setButton(Qt::LeftButton);
    releaseEvent.setScenePos(item_level0->mapToScene(5, 5));
    releaseEvent.setScreenPos(view.mapFromScene(pressEvent.scenePos()));
    QApplication::sendEvent(&scene, &pressEvent);
    QApplication::sendEvent(&scene, &releaseEvent);

    QCOMPARE(item_level0->numMouseEvents(), 2);

    // Send events to a level1 item
    pressEvent.setScenePos(item1_level1->mapToScene(5, 5));
    pressEvent.setScreenPos(view.mapFromScene(pressEvent.scenePos()));
    releaseEvent.setScenePos(item1_level1->mapToScene(5, 5));
    releaseEvent.setScreenPos(view.mapFromScene(releaseEvent.scenePos()));
    QApplication::sendEvent(&scene, &pressEvent);
    QApplication::sendEvent(&scene, &releaseEvent);

    QCOMPARE(item_level0->numMouseEvents(), 2);
    QCOMPARE(item1_level1->numMouseEvents(), 2);

    // Send events to a level2 item
    pressEvent.setScenePos(item1_level2->mapToScene(5, 5));
    pressEvent.setScreenPos(view.mapFromScene(pressEvent.scenePos()));
    releaseEvent.setScenePos(item1_level2->mapToScene(5, 5));
    releaseEvent.setScreenPos(view.mapFromScene(releaseEvent.scenePos()));
    QApplication::sendEvent(&scene, &pressEvent);
    QApplication::sendEvent(&scene, &releaseEvent);

    QCOMPARE(item_level0->numMouseEvents(), 2);
    QCOMPARE(item1_level1->numMouseEvents(), 2);
    QCOMPARE(item1_level2->numMouseEvents(), 2);

    item_level0->setHandlesChildEvents(true);

    // Send events to a level1 item
    pressEvent.setScenePos(item1_level1->mapToScene(5, 5));
    pressEvent.setScreenPos(view.mapFromScene(pressEvent.scenePos()));
    releaseEvent.setScenePos(item1_level1->mapToScene(5, 5));
    releaseEvent.setScreenPos(view.mapFromScene(releaseEvent.scenePos()));
    QApplication::sendEvent(&scene, &pressEvent);
    QApplication::sendEvent(&scene, &releaseEvent);

    QCOMPARE(item_level0->numMouseEvents(), 4);
    QCOMPARE(item1_level1->numMouseEvents(), 2);

    // Send events to a level2 item
    pressEvent.setScenePos(item1_level2->mapToScene(5, 5));
    pressEvent.setScreenPos(view.mapFromScene(pressEvent.scenePos()));
    releaseEvent.setScenePos(item1_level2->mapToScene(5, 5));
    releaseEvent.setScreenPos(view.mapFromScene(releaseEvent.scenePos()));
    QApplication::sendEvent(&scene, &pressEvent);
    QApplication::sendEvent(&scene, &releaseEvent);

    QCOMPARE(item_level0->numMouseEvents(), 6);
    QCOMPARE(item1_level1->numMouseEvents(), 2);
    QCOMPARE(item1_level2->numMouseEvents(), 2);

    item_level0->setHandlesChildEvents(false);

    // Send events to a level1 item
    pressEvent.setScenePos(item1_level1->mapToScene(5, 5));
    pressEvent.setScreenPos(view.mapFromScene(pressEvent.scenePos()));
    releaseEvent.setScenePos(item1_level1->mapToScene(5, 5));
    releaseEvent.setScreenPos(view.mapFromScene(releaseEvent.scenePos()));
    QApplication::sendEvent(&scene, &pressEvent);
    QApplication::sendEvent(&scene, &releaseEvent);

    QCOMPARE(item_level0->numMouseEvents(), 6);
    QCOMPARE(item1_level1->numMouseEvents(), 4);

    // Send events to a level2 item
    pressEvent.setScenePos(item1_level2->mapToScene(5, 5));
    pressEvent.setScreenPos(view.mapFromScene(pressEvent.scenePos()));
    releaseEvent.setScenePos(item1_level2->mapToScene(5, 5));
    releaseEvent.setScreenPos(view.mapFromScene(releaseEvent.scenePos()));
    QApplication::sendEvent(&scene, &pressEvent);
    QApplication::sendEvent(&scene, &releaseEvent);

    QCOMPARE(item_level0->numMouseEvents(), 6);
    QCOMPARE(item1_level1->numMouseEvents(), 4);
    QCOMPARE(item1_level2->numMouseEvents(), 4);
}

void tst_QGraphicsItem::handlesChildEvents2()
{
    ChildEventTester *root = new ChildEventTester(QRectF(0, 0, 10, 10));
    root->setHandlesChildEvents(true);
    QVERIFY(root->handlesChildEvents());

    ChildEventTester *child = new ChildEventTester(QRectF(0, 0, 10, 10), root);
    QVERIFY(!child->handlesChildEvents());

    ChildEventTester *child2 = new ChildEventTester(QRectF(0, 0, 10, 10));
    ChildEventTester *child3 = new ChildEventTester(QRectF(0, 0, 10, 10), child2);
    ChildEventTester *child4 = new ChildEventTester(QRectF(0, 0, 10, 10), child3);
    child2->setParentItem(root);
    QVERIFY(!child2->handlesChildEvents());
    QVERIFY(!child3->handlesChildEvents());
    QVERIFY(!child4->handlesChildEvents());

    QGraphicsScene scene;
    scene.addItem(root);

    QGraphicsView view(&scene);
    view.show();

    QTestEventLoop::instance().enterLoop(1);

    QMouseEvent event(QEvent::MouseButtonPress, view.mapFromScene(5, 5),
                      view.viewport()->mapToGlobal(view.mapFromScene(5, 5)), Qt::LeftButton, 0, 0);
    QApplication::sendEvent(view.viewport(), &event);

    QCOMPARE(root->numMouseEvents(), 1);
}

class CustomItem : public QGraphicsItem
{
public:
    QRectF boundingRect() const
    { return QRectF(-110, -110, 220, 220); }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        for (int x = -100; x <= 100; x += 25)
            painter->drawLine(x, -100, x, 100);
        for (int y = -100; y <= 100; y += 25)
            painter->drawLine(-100, y, 100, y);

        QFont font = painter->font();
        font.setPointSize(4);
        painter->setFont(font);
        for (int x = -100; x < 100; x += 25) {
            for (int y = -100; y < 100; y += 25) {
                painter->drawText(QRectF(x, y, 25, 25), Qt::AlignCenter, QString("%1x%2").arg(x).arg(y));
            }
        }
    }
};

void tst_QGraphicsItem::ensureVisible()
{
    QGraphicsScene scene;
    scene.setSceneRect(-200, -200, 400, 400);
    QGraphicsItem *item = new CustomItem;
    scene.addItem(item);

    QGraphicsView view(&scene);
    view.setFixedSize(300, 300);
    view.show();

    for (int i = 0; i < 25; ++i) {
        view.scale(1.06, 1.06);
        QTest::qWait(25);
    }

    item->ensureVisible(-100, -100, 25, 25);
    QTest::qWait(250);
    
    for (int x = -100; x < 100; x += 25) {
        for (int y = -100; y < 100; y += 25) {
            int xmargin = rand() % 75;
            int ymargin = rand() % 75;
            item->ensureVisible(x, y, 25, 25, xmargin, ymargin);
            QTest::qWait(25);

            QPolygonF viewScenePoly;
            viewScenePoly << view.mapToScene(view.rect().topLeft())
                          << view.mapToScene(view.rect().topRight())
                          << view.mapToScene(view.rect().bottomRight())
                          << view.mapToScene(view.rect().bottomLeft());

            QVERIFY(scene.items(viewScenePoly).contains(item));

            QPainterPath path;
            path.addPolygon(viewScenePoly);
            QVERIFY(path.contains(item->mapToScene(x + 12, y + 12)));

            QPolygonF viewScenePolyMinusMargins;
            viewScenePolyMinusMargins << view.mapToScene(view.rect().topLeft() + QPoint(xmargin, ymargin))
                          << view.mapToScene(view.rect().topRight() + QPoint(-xmargin, ymargin))
                          << view.mapToScene(view.rect().bottomRight() + QPoint(-xmargin, -ymargin))
                          << view.mapToScene(view.rect().bottomLeft() + QPoint(xmargin, -ymargin));

            QPainterPath path2;
            path2.addPolygon(viewScenePolyMinusMargins);
            QVERIFY(path2.contains(item->mapToScene(x + 12, y + 12)));
        }
    }

    item->ensureVisible(100, 100, 25, 25);
    QTest::qWait(250);
}

void tst_QGraphicsItem::cursor()
{
    QGraphicsScene scene;
    QGraphicsRectItem *item1 = scene.addRect(QRectF(0, 0, 50, 50));
    QGraphicsRectItem *item2 = scene.addRect(QRectF(0, 0, 50, 50));
    item1->setPos(-100, 0);
    item2->setPos(50, 0);

    QVERIFY(!item1->hasCursor());
    QVERIFY(!item2->hasCursor());

    item1->setCursor(Qt::IBeamCursor);
    item2->setCursor(Qt::PointingHandCursor);

    QVERIFY(item1->hasCursor());
    QVERIFY(item2->hasCursor());

    item1->setCursor(QCursor());
    item2->setCursor(QCursor());

    QVERIFY(item1->hasCursor());
    QVERIFY(item2->hasCursor());

    item1->unsetCursor();
    item2->unsetCursor();

    QVERIFY(!item1->hasCursor());
    QVERIFY(!item2->hasCursor());

    item1->setCursor(Qt::IBeamCursor);
    item2->setCursor(Qt::PointingHandCursor);

    QGraphicsView view(&scene);
    view.setFixedSize(200, 100);
    view.show();
    QTest::mouseMove(&view, view.rect().center());

    QTest::qWait(250);

    QCursor cursor = view.viewport()->cursor();

    {
        QMouseEvent event(QEvent::MouseMove, QPoint(100, 50), Qt::NoButton, 0, 0);
        QApplication::sendEvent(view.viewport(), &event);
    }

    QTest::qWait(250);

    QCOMPARE(view.viewport()->cursor().shape(), cursor.shape());

    {
        QMouseEvent event(QEvent::MouseMove, QPoint(25, 25), Qt::NoButton, 0, 0);
        QApplication::sendEvent(view.viewport(), &event);
    }

    QTest::qWait(250);

    QCOMPARE(view.viewport()->cursor().shape(), item1->cursor().shape());

    {
        QMouseEvent event(QEvent::MouseMove, QPoint(175, 25), Qt::NoButton, 0, 0);
        QApplication::sendEvent(view.viewport(), &event);
    }
    
    QTest::qWait(250);

    QCOMPARE(view.viewport()->cursor().shape(), item2->cursor().shape());

    {
        QMouseEvent event(QEvent::MouseMove, QPoint(100, 25), Qt::NoButton, 0, 0);
        QApplication::sendEvent(view.viewport(), &event);
    }

    QTest::qWait(250);

    QCOMPARE(view.viewport()->cursor().shape(), cursor.shape());
}
/*
void tst_QGraphicsItem::textControlGetterSetter()
{
    QGraphicsTextItem *item = new QGraphicsTextItem;
    QVERIFY(item->textControl()->parent() == item);
    QPointer<QTextControl> control = item->textControl();
    delete item;
    QVERIFY(!control);

    item = new QGraphicsTextItem;

    QPointer<QTextControl> oldControl = control;
    control = new QTextControl;

    item->setTextControl(control);
    QVERIFY(item->textControl() == control);
    QVERIFY(!control->parent());
    QVERIFY(!oldControl);

    // set some text to give it a size, to test that
    // setTextControl (re)connects signals
    const QRectF oldBoundingRect = item->boundingRect();
    QVERIFY(oldBoundingRect.isValid());
    item->setPlainText("Some text");
    item->adjustSize();
    QVERIFY(item->boundingRect().isValid());
    QVERIFY(item->boundingRect() != oldBoundingRect);
    
    // test that on setting a control the item size
    // is adjusted
    oldControl = control;
    control = new QTextControl;
    control->setPlainText("foo!");
    item->setTextControl(control);
    QCOMPARE(item->boundingRect().size(), control->document()->documentLayout()->documentSize());
    
    QVERIFY(oldControl);
    delete oldControl;

    delete item;
    QVERIFY(control);
    delete control;
}
*/
void tst_QGraphicsItem::defaultItemTest_QGraphicsPixmapItem()
{
    QGraphicsPixmapItem item;
    QVERIFY(item.pixmap().isNull());
    QCOMPARE(item.offset(), QPointF());
    QCOMPARE(item.transformationMode(), Qt::FastTransformation);

    QPixmap pixmap(300, 200);
    pixmap.fill(Qt::red);
    item.setPixmap(pixmap);
    QCOMPARE(item.pixmap(), pixmap);

    item.setTransformationMode(Qt::FastTransformation);
    QCOMPARE(item.transformationMode(), Qt::FastTransformation);
    item.setTransformationMode(Qt::SmoothTransformation);
    QCOMPARE(item.transformationMode(), Qt::SmoothTransformation);

    item.setOffset(-15, -15);
    QCOMPARE(item.offset(), QPointF(-15, -15));
    item.setOffset(QPointF(-10, -10));
    QCOMPARE(item.offset(), QPointF(-10, -10));

    QCOMPARE(item.boundingRect(), QRectF(-10.5, -10.5, 301.5, 201.5));
}

class ItemChangeTester : public QGraphicsRectItem
{
public:
    QVariant itemChangeReturnValue;
    QList<GraphicsItemChange> changes;
    QList<QVariant> values;
    QList<QVariant> oldValues;
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value)
    {
        changes << change;
        values << value;
        switch (change) {
        case QGraphicsItem::ItemPositionChange:
            oldValues << pos();
            break;
        case QGraphicsItem::ItemPositionHasChanged:
            break;
        case QGraphicsItem::ItemMatrixChange: {
            QVariant variant;
            qVariantSetValue<QMatrix>(variant, matrix());
            oldValues << variant;
        }
            break;
        case QGraphicsItem::ItemTransformChange: {
            QVariant variant;
            qVariantSetValue<QTransform>(variant, transform());
            oldValues << variant;
        }
            break;
        case QGraphicsItem::ItemTransformHasChanged:
            break;
        case QGraphicsItem::ItemVisibleChange:
            oldValues << isVisible();
            break;
        case QGraphicsItem::ItemEnabledChange:
            oldValues << isEnabled();
            break;
        case QGraphicsItem::ItemSelectedChange:
            oldValues << isSelected();
            break;
        case QGraphicsItem::ItemParentChange:
            oldValues << parentItem();
            break;
        case QGraphicsItem::ItemChildAddedChange:
            oldValues << children().size();
            break;
        case QGraphicsItem::ItemChildRemovedChange:
            oldValues << children().size();
            break;
        }
        return itemChangeReturnValue.isValid() ? itemChangeReturnValue : value;
    }
};

void tst_QGraphicsItem::itemChange()
{
    ItemChangeTester tester;
    ItemChangeTester testerHelper;
    QVERIFY(tester.changes.isEmpty());
    QVERIFY(tester.values.isEmpty());

    int changeCount = 0;
    {
        // ItemEnabledChange
        tester.itemChangeReturnValue = true;
        tester.setEnabled(false);
        QCOMPARE(tester.changes.size(), ++changeCount);
        QCOMPARE(tester.changes.last(), QGraphicsItem::ItemEnabledChange);
        QCOMPARE(tester.values.last(), QVariant(false));
        QCOMPARE(tester.oldValues.last(), QVariant(true));
        QCOMPARE(tester.isEnabled(), true);
    }
    {
        // ItemMatrixChange / ItemTransformHasChanged
        qVariantSetValue<QMatrix>(tester.itemChangeReturnValue, QMatrix().rotate(90));
        tester.setMatrix(QMatrix().translate(50, 0), true);
        ++changeCount; // notification sent too
        QCOMPARE(tester.changes.size(), ++changeCount);
        QCOMPARE(int(tester.changes.at(tester.changes.size() - 2)), int(QGraphicsItem::ItemMatrixChange));
        QCOMPARE(int(tester.changes.last()), int(QGraphicsItem::ItemTransformHasChanged));
        QCOMPARE(qVariantValue<QMatrix>(tester.values.at(tester.values.size() - 2)),
                 QMatrix().translate(50, 0));
        QCOMPARE(tester.values.last(), QVariant(QTransform(QMatrix().rotate(90))));
        QVariant variant;
        qVariantSetValue<QMatrix>(variant, QMatrix());
        QCOMPARE(tester.oldValues.last(), variant);
        QCOMPARE(tester.matrix(), QMatrix().rotate(90));
    }
    {
        tester.resetTransform();
        ++changeCount;
        ++changeCount; // notification sent too

        // ItemTransformChange / ItemTransformHasChanged
        qVariantSetValue<QTransform>(tester.itemChangeReturnValue, QTransform().rotate(90));
        tester.translate(50, 0);
        ++changeCount; // notification sent too
        QCOMPARE(tester.changes.size(), ++changeCount);
        QCOMPARE(tester.changes.at(tester.changes.size() - 2), QGraphicsItem::ItemTransformChange);
        QCOMPARE(tester.changes.last(), QGraphicsItem::ItemTransformHasChanged);
        QCOMPARE(qVariantValue<QTransform>(tester.values.at(tester.values.size() - 2)),
                 QTransform().translate(50, 0));
        QCOMPARE(tester.values.last(), QVariant(QTransform().rotate(90)));
        QVariant variant;
        qVariantSetValue<QTransform>(variant, QTransform());
        QCOMPARE(tester.oldValues.last(), variant);
        QCOMPARE(tester.transform(), QTransform().rotate(90));
    }
    {
        // ItemPositionChange / ItemPositionHasChanged
        tester.itemChangeReturnValue = QPointF(42, 0);
        tester.setPos(0, 42);
        ++changeCount; // notification sent too
        QCOMPARE(tester.changes.size(), ++changeCount);
        QCOMPARE(tester.changes.at(tester.changes.size() - 2), QGraphicsItem::ItemPositionChange);
        QCOMPARE(tester.changes.last(), QGraphicsItem::ItemPositionHasChanged);
        QCOMPARE(tester.values.at(tester.changes.size() - 2), QVariant(QPointF(0, 42)));
        QCOMPARE(tester.values.last(), QVariant(QPointF(42, 0)));
        QCOMPARE(tester.oldValues.last(), QVariant(QPointF()));
        QCOMPARE(tester.pos(), QPointF(42, 0));
    }
    {
        // ItemSelectedChange
        tester.setSelected(true);
        QCOMPARE(tester.changes.size(), changeCount); // No change :-)
        tester.setFlag(QGraphicsItem::ItemIsSelectable);
        tester.itemChangeReturnValue = false;
        tester.setSelected(true);
        QCOMPARE(tester.changes.size(), ++changeCount);
        QCOMPARE(tester.changes.last(), QGraphicsItem::ItemSelectedChange);
        QCOMPARE(tester.values.last(), QVariant(true));
        QCOMPARE(tester.oldValues.last(), QVariant(false));
        QCOMPARE(tester.isSelected(), false);
    }
    {
        // ItemVisibleChange
        tester.itemChangeReturnValue = true;
        tester.setVisible(false);
        QCOMPARE(tester.changes.size(), ++changeCount);
        QCOMPARE(tester.changes.last(), QGraphicsItem::ItemVisibleChange);
        QCOMPARE(tester.values.last(), QVariant(false));
        QCOMPARE(tester.isVisible(), true);
    }
    {
        // ItemParentChange
        qVariantSetValue<QGraphicsItem *>(tester.itemChangeReturnValue, 0);
        tester.setParentItem(&testerHelper);
        QCOMPARE(tester.changes.size(), ++changeCount);
        QCOMPARE(tester.changes.last(), QGraphicsItem::ItemParentChange);
        QCOMPARE(qVariantValue<QGraphicsItem *>(tester.values.last()), (QGraphicsItem *)&testerHelper);
        QCOMPARE(qVariantValue<QGraphicsItem *>(tester.oldValues.last()), (QGraphicsItem *)0);
        QCOMPARE(tester.parentItem(), (QGraphicsItem *)0);
    }
    {
        // ItemChildAddedChange
        tester.itemChangeReturnValue.clear();
        testerHelper.setParentItem(&tester);
        QCOMPARE(tester.changes.size(), ++changeCount);
        QCOMPARE(tester.changes.last(), QGraphicsItem::ItemChildAddedChange);
        QCOMPARE(qVariantValue<QGraphicsItem *>(tester.values.last()), (QGraphicsItem *)&testerHelper);
    }
    {
        // ItemChildRemovedChange 1
        testerHelper.setParentItem(0);
        QCOMPARE(tester.changes.size(), ++changeCount);
        QCOMPARE(tester.changes.last(), QGraphicsItem::ItemChildRemovedChange);
        QCOMPARE(qVariantValue<QGraphicsItem *>(tester.values.last()), (QGraphicsItem *)&testerHelper);
    }
    {
        // ItemChildRemovedChange 2
        ItemChangeTester parent;
        ItemChangeTester *child = new ItemChangeTester;
        child->setParentItem(&parent);
        QCOMPARE(parent.changes.last(), QGraphicsItem::ItemChildAddedChange);
        QCOMPARE(qVariantValue<QGraphicsItem *>(parent.values.last()), (QGraphicsItem *)child); 
        delete child;
        QCOMPARE(parent.changes.last(), QGraphicsItem::ItemChildRemovedChange);
        QCOMPARE(qVariantValue<QGraphicsItem *>(parent.values.last()), (QGraphicsItem *)child); 
    }
}

class EventFilterTesterItem : public QGraphicsLineItem
{
public:
    QList<QEvent::Type> filteredEvents;
    QList<QGraphicsItem *> filteredEventReceivers;
    bool handlesSceneEvents;

    QList<QEvent::Type> receivedEvents;

protected:
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event)
    {
        filteredEvents << event->type();
        filteredEventReceivers << watched;
        return handlesSceneEvents;
    }

    bool sceneEvent(QEvent *event)
    {
        return QGraphicsLineItem::sceneEvent(event);
    }
};

void tst_QGraphicsItem::sceneEventFilter()
{
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.show();
    
    QGraphicsTextItem *text1 = scene.addText(QLatin1String("Text1"));
    QGraphicsTextItem *text2 = scene.addText(QLatin1String("Text2"));
    QGraphicsTextItem *text3 = scene.addText(QLatin1String("Text3"));
    text1->setFlag(QGraphicsItem::ItemIsFocusable);
    text2->setFlag(QGraphicsItem::ItemIsFocusable);
    text3->setFlag(QGraphicsItem::ItemIsFocusable);

    EventFilterTesterItem *tester = new EventFilterTesterItem;
    scene.addItem(tester);

    QVERIFY(!text1->hasFocus());
    text1->installSceneEventFilter(tester);
    text1->setFocus();
    QVERIFY(text1->hasFocus());

    QCOMPARE(tester->filteredEvents.size(), 1);
    QCOMPARE(tester->filteredEvents.at(0), QEvent::FocusIn);
    QCOMPARE(tester->filteredEventReceivers.at(0), text1);

    text2->installSceneEventFilter(tester);
    text3->installSceneEventFilter(tester);

    text2->setFocus();
    text3->setFocus();

    QCOMPARE(tester->filteredEvents.size(), 5);
    QCOMPARE(tester->filteredEvents.at(1), QEvent::FocusOut);
    QCOMPARE(tester->filteredEventReceivers.at(1), text1);
    QCOMPARE(tester->filteredEvents.at(2), QEvent::FocusIn);
    QCOMPARE(tester->filteredEventReceivers.at(2), text2);
    QCOMPARE(tester->filteredEvents.at(3), QEvent::FocusOut);
    QCOMPARE(tester->filteredEventReceivers.at(3), text2);
    QCOMPARE(tester->filteredEvents.at(4), QEvent::FocusIn);
    QCOMPARE(tester->filteredEventReceivers.at(4), text3);

    text1->removeSceneEventFilter(tester);
    text1->setFocus();

    QCOMPARE(tester->filteredEvents.size(), 6);
    QCOMPARE(tester->filteredEvents.at(5), QEvent::FocusOut);
    QCOMPARE(tester->filteredEventReceivers.at(5), text3);

    tester->handlesSceneEvents = true;
    text2->setFocus();

    QCOMPARE(tester->filteredEvents.size(), 7);
    QCOMPARE(tester->filteredEvents.at(6), QEvent::FocusIn);
    QCOMPARE(tester->filteredEventReceivers.at(6), text2);

    QVERIFY(text2->hasFocus());
}

class GeometryChanger : public QGraphicsItem
{
public:
    void changeGeometry()
    { prepareGeometryChange(); }
};

void tst_QGraphicsItem::prepareGeometryChange()
{
    {
        QGraphicsScene scene;
        QGraphicsItem *item = scene.addRect(QRectF(0, 0, 100, 100));
        QVERIFY(scene.items(QRectF(0, 0, 100, 100)).contains(item));
        ((GeometryChanger *)item)->changeGeometry();
        QVERIFY(scene.items(QRectF(0, 0, 100, 100)).contains(item));
    }
}


class PaintTester : public QGraphicsRectItem
{
public:
    PaintTester() : widget(NULL) { setRect(QRectF(10, 10, 20, 20)); }

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *w)
    {
        widget = w;
    }
    
    QWidget*  widget;
};

void tst_QGraphicsItem::paint()
{
    QGraphicsScene scene;

    PaintTester paintTester;
    scene.addItem(&paintTester);

    QGraphicsView view(&scene);
    
    view.show();
    QTest::qWait(250);

    QVERIFY(paintTester.widget == view.viewport());
}

class HarakiriItem : public QGraphicsRectItem
{
public:
    HarakiriItem(int harakiriPoint)
        : QGraphicsRectItem(QRectF(0, 0, 100, 100)), harakiri(harakiriPoint)
    { dead = 0; }
    
    static int dead;
    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        QGraphicsRectItem::paint(painter, option, widget);
        if (harakiri == 0) {
            dead = 1;
            delete this;
        }
    }

    void advance(int n)
    {
        if (harakiri == 1 && n == 0) {
            // delete unsupported
            /*
            dead = 1;
            delete this;
            */
        }
        if (harakiri == 2 && n == 1) {
            dead = 1;
            delete this;
        }
    }

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *)
    {
        if (harakiri == 3) {
            dead = 1;
            delete this;
        }
    }

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event)
    {
        // ??
        return QGraphicsRectItem::dragEnterEvent(event);
    }

    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
    {
        // ??
        return QGraphicsRectItem::dragLeaveEvent(event);
    }

    void dragMoveEvent(QGraphicsSceneDragDropEvent *event)
    {
        // ??
        return QGraphicsRectItem::dragMoveEvent(event);
    }

    void dropEvent(QGraphicsSceneDragDropEvent *event)
    {
        // ??
        return QGraphicsRectItem::dropEvent(event);
    }

    void focusInEvent(QFocusEvent *)
    {
        if (harakiri == 4) {
            dead = 1;
            delete this;
        }
    }

    void focusOutEvent(QFocusEvent *)
    {
        if (harakiri == 5) {
            dead = 1;
            delete this;
        }
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent *)
    {
        if (harakiri == 6) {
            dead = 1;
            delete this;
        }
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *)
    {
        if (harakiri == 7) {
            dead = 1;
            delete this;
        }
    }

    void hoverMoveEvent(QGraphicsSceneHoverEvent *)
    {
        if (harakiri == 8) {
            dead = 1;
            delete this;
        }
    }

    void inputMethodEvent(QInputMethodEvent *event)
    {
        // ??
        return QGraphicsRectItem::inputMethodEvent(event);
    }

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const
    {
        // ??
        return QGraphicsRectItem::inputMethodQuery(query);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value)
    {
        // deletion not supported
        return QGraphicsRectItem::itemChange(change, value);
    }

    void keyPressEvent(QKeyEvent *)
    {
        if (harakiri == 9) {
            dead = 1;
            delete this;
        }
    }

    void keyReleaseEvent(QKeyEvent *)
    {
        if (harakiri == 10) {
            dead = 1;
            delete this;
        }
    }

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
    {
        if (harakiri == 11) {
            dead = 1;
            delete this;
        }
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *)
    {
        if (harakiri == 12) {
            dead = 1;
            delete this;
        }
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *)
    {
        if (harakiri == 13) {
            dead = 1;
            delete this;
        }
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *)
    {
        if (harakiri == 14) {
            dead = 1;
            delete this;
        }
    }

    bool sceneEvent(QEvent *event)
    {
        // deletion not supported
        return QGraphicsRectItem::sceneEvent(event);
    }

    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event)
    {
        // deletion not supported
        return QGraphicsRectItem::sceneEventFilter(watched, event);
    }

    void wheelEvent(QGraphicsSceneWheelEvent *) 
    {
        if (harakiri == 16) {
            dead = 1;
            delete this;
        }
    }

private:
    int harakiri;
};

int HarakiriItem::dead;

void tst_QGraphicsItem::deleteItemInEventHandlers()
{
    for (int i = 0; i < 17; ++i) {
        QGraphicsScene scene;
        HarakiriItem *item = new HarakiriItem(i);
        item->setAcceptsHoverEvents(true);
        item->setFlag(QGraphicsItem::ItemIsFocusable);

        scene.addItem(item);

        item->installSceneEventFilter(item); // <- ehey!
        
        QGraphicsView view(&scene);
        view.show();

        qApp->processEvents();
        qApp->processEvents();

        if (!item->dead)
            scene.advance();

        if (!item->dead)
            QTest::mouseMove(view.viewport(), view.mapFromScene(item->scenePos()));
        if (!item->dead)
            QTest::mouseClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item->scenePos()));
        if (!item->dead)
            QTest::mouseDClick(view.viewport(), Qt::LeftButton, 0, view.mapFromScene(item->scenePos()));
        if (!item->dead)
            QTest::mouseClick(view.viewport(), Qt::RightButton, 0, view.mapFromScene(item->scenePos()));
        if (!item->dead)
            QTest::mouseMove(view.viewport(), view.mapFromScene(item->scenePos() + QPointF(20, -20)));
        if (!item->dead)
            item->setFocus();
        if (!item->dead)
            item->clearFocus();
        if (!item->dead)
            item->setFocus();
        if (!item->dead)
            QTest::keyPress(view.viewport(), Qt::Key_A);
        if (!item->dead)
            QTest::keyRelease(view.viewport(), Qt::Key_A);
        if (!item->dead)
            QTest::keyPress(view.viewport(), Qt::Key_A);
        if (!item->dead)
            QTest::keyRelease(view.viewport(), Qt::Key_A);
    }
}

class ItemPaintsOutsideShape : public QGraphicsItem
{
public:
    QRectF boundingRect() const
    {
        return QRectF(0, 0, 100, 100);
    }
    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        painter->fillRect(-50, -50, 200, 200, Qt::red);
        painter->fillRect(0, 0, 100, 100, Qt::blue);
    }
};

void tst_QGraphicsItem::itemClipsToShape()
{
    QGraphicsItem *clippedItem = new ItemPaintsOutsideShape;
    clippedItem->setFlag(QGraphicsItem::ItemClipsToShape);

    QGraphicsItem *unclippedItem = new ItemPaintsOutsideShape;
    unclippedItem->setPos(200, 0);

    QGraphicsScene scene(-50, -50, 400, 200);
    scene.addItem(clippedItem);
    scene.addItem(unclippedItem);

    QImage image(400, 200, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    scene.render(&painter);
    painter.end();

    QCOMPARE(image.pixel(45, 100), QRgb(0));
    QCOMPARE(image.pixel(100, 45), QRgb(0));
    QCOMPARE(image.pixel(155, 100), QRgb(0));
    QCOMPARE(image.pixel(45, 155), QRgb(0));
    QCOMPARE(image.pixel(55, 100), QColor(Qt::blue).rgba());
    QCOMPARE(image.pixel(100, 55), QColor(Qt::blue).rgba());
    QCOMPARE(image.pixel(145, 100), QColor(Qt::blue).rgba());
    QCOMPARE(image.pixel(55, 145), QColor(Qt::blue).rgba());
    QCOMPARE(image.pixel(245, 100), QColor(Qt::red).rgba());
    QCOMPARE(image.pixel(300, 45), QColor(Qt::red).rgba());
    QCOMPARE(image.pixel(355, 100), QColor(Qt::red).rgba());
    QCOMPARE(image.pixel(245, 155), QColor(Qt::red).rgba());
    QCOMPARE(image.pixel(255, 100), QColor(Qt::blue).rgba());
    QCOMPARE(image.pixel(300, 55), QColor(Qt::blue).rgba());
    QCOMPARE(image.pixel(345, 100), QColor(Qt::blue).rgba());
    QCOMPARE(image.pixel(255, 145), QColor(Qt::blue).rgba());
    
    image.save("itemClipsToShape.png");
}

void tst_QGraphicsItem::itemClipsChildrenToShape()
{
    QGraphicsScene scene;
    QGraphicsItem *rect = scene.addRect(0, 0, 50, 50, QPen(Qt::NoPen), QBrush(Qt::yellow));

    QGraphicsItem *ellipse = scene.addEllipse(0, 0, 100, 100, QPen(Qt::NoPen), QBrush(Qt::green));
    ellipse->setParentItem(rect);
    
    QGraphicsItem *clippedEllipse = scene.addEllipse(0, 0, 50, 50, QPen(Qt::NoPen), QBrush(Qt::blue));
    clippedEllipse->setParentItem(ellipse);

    QGraphicsItem *clippedEllipse2 = scene.addEllipse(0, 0, 25, 25, QPen(Qt::NoPen), QBrush(Qt::red));
    clippedEllipse2->setParentItem(clippedEllipse);

    QVERIFY(!(ellipse->flags() & QGraphicsItem::ItemClipsChildrenToShape));
    ellipse->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    QVERIFY((ellipse->flags() & QGraphicsItem::ItemClipsChildrenToShape));

    QImage image(100, 100, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    scene.render(&painter);
    painter.end();

    QCOMPARE(image.pixel(16, 16), QColor(255, 0, 0).rgba());
    QCOMPARE(image.pixel(32, 32), QColor(0, 0, 255).rgba());
    QCOMPARE(image.pixel(50, 50), QColor(0, 255, 0).rgba());
    QCOMPARE(image.pixel(12, 12), QColor(255, 255, 0).rgba());

    image.save("itemClipsChildrenToShape.png");
}

class ItemAddScene : public QGraphicsScene
{
    Q_OBJECT
public:
    ItemAddScene()
    {
        QTimer::singleShot(500, this, SLOT(newTextItem()));
    }

public slots:
    void newTextItem()
    {
        // Add a text item
        QGraphicsItem *item = new QGraphicsTextItem("This item will not ensure that it's visible", 0, this);
        item->setPos(.0, .0);
        item->show();
    }
};

void tst_QGraphicsItem::task141694_textItemEnsureVisible()
{
    ItemAddScene scene;
    scene.setSceneRect(-1000, -1000, 2000, 2000);

    QGraphicsView view(&scene);
    view.setFixedSize(200, 200);
    view.show();

    view.ensureVisible(-1000, -1000, 5, 5);
    int hscroll = view.horizontalScrollBar()->value();
    int vscroll = view.verticalScrollBar()->value();

    QTestEventLoop::instance().enterLoop(1);

    // This should not cause the view to scroll
    QCOMPARE(view.horizontalScrollBar()->value(), hscroll);
    QCOMPARE(view.verticalScrollBar()->value(), vscroll);
}

QTEST_MAIN(tst_QGraphicsItem)
#include "tst_qgraphicsitem.moc"
#endif
