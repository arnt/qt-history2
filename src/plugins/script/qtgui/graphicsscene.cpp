#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsScene>
#include <qdebug.h>
#include "../global.h"

Q_DECLARE_METATYPE(QGraphicsItem*)
Q_DECLARE_METATYPE(QGraphicsItemGroup*)
Q_DECLARE_METATYPE(QGraphicsScene*)
Q_DECLARE_METATYPE(QGraphicsEllipseItem*)
Q_DECLARE_METATYPE(QGraphicsLineItem*)
Q_DECLARE_METATYPE(QGraphicsPathItem*)
Q_DECLARE_METATYPE(QGraphicsPixmapItem*)
Q_DECLARE_METATYPE(QGraphicsPolygonItem*)
Q_DECLARE_METATYPE(QGraphicsRectItem*)
Q_DECLARE_METATYPE(QGraphicsSimpleTextItem*)
Q_DECLARE_METATYPE(QGraphicsTextItem*)
Q_DECLARE_METATYPE(QList<QGraphicsItem*>)
Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QPolygonF)
Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(QGraphicsView*)
Q_DECLARE_METATYPE(QList<QGraphicsView*>)

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QGraphicsScene *scene = 0;
    if (ctx->argumentCount() > 2) {
        scene = new QGraphicsScene(ctx->argument(0).toNumber(),
                                   ctx->argument(1).toNumber(),
                                   ctx->argument(2).toNumber(),
                                   ctx->argument(3).toNumber(),
                                   ctx->argument(4).toQObject());
    } else if (ctx->argumentCount() > 1) {
        scene = new QGraphicsScene(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                                   ctx->argument(1).toQObject());
    } else {
        scene = new QGraphicsScene(ctx->argument(0).toQObject());
    }
    return eng->newQObject(scene, QScriptEngine::AutoOwnership);
}


/////////////////////////////////////////////////////////////

static QScriptValue addEllipse(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, addEllipse);
    if (ctx->argumentCount() >= 4) {
        return eng->toScriptValue(self->addEllipse(ctx->argument(0).toNumber(),
                                                   ctx->argument(1).toNumber(),
                                                   ctx->argument(2).toNumber(),
                                                   ctx->argument(3).toNumber(),
                                                   qscriptvalue_cast<QPen>(ctx->argument(4)),
                                                   qscriptvalue_cast<QBrush>(ctx->argument(5))));
    } else {
        return eng->toScriptValue(self->addEllipse(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                                                   qscriptvalue_cast<QPen>(ctx->argument(1)),
                                                   qscriptvalue_cast<QBrush>(ctx->argument(2))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue addItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, addItem);
    QScriptValue arg = ctx->argument(0);
    QGraphicsItem *item = qscriptvalue_cast<QGraphicsItem*>(arg);
    self->addItem(item);
    if (item)
        QScript::maybeReleaseOwnership(arg);
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue addLine(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, addLine);
    if (ctx->argumentCount() >= 4) {
        return eng->toScriptValue(self->addLine(ctx->argument(0).toNumber(),
                                                ctx->argument(1).toNumber(),
                                                ctx->argument(2).toNumber(),
                                                ctx->argument(3).toNumber(),
                                                qscriptvalue_cast<QPen>(ctx->argument(4))));
    } else {
        return eng->toScriptValue(self->addLine(qscriptvalue_cast<QLineF>(ctx->argument(0)),
                                                qscriptvalue_cast<QPen>(ctx->argument(1))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue addPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, addPath);
    return eng->toScriptValue(self->addPath(qscriptvalue_cast<QPainterPath>(ctx->argument(0)),
                                            qscriptvalue_cast<QPen>(ctx->argument(1)),
                                            qscriptvalue_cast<QBrush>(ctx->argument(2))));
                                            
}

/////////////////////////////////////////////////////////////

static QScriptValue addPixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, addPixmap);
    return eng->toScriptValue(self->addPixmap(qscriptvalue_cast<QPixmap>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue addPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, addPolygon);
    return eng->toScriptValue(self->addPolygon(qscriptvalue_cast<QPolygonF>(ctx->argument(0)),
                                               qscriptvalue_cast<QPen>(ctx->argument(1)),
                                               qscriptvalue_cast<QBrush>(ctx->argument(2))));
}

/////////////////////////////////////////////////////////////

static QScriptValue addRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, addRect);
    if (ctx->argumentCount() >= 4) {
        return eng->toScriptValue(self->addRect(ctx->argument(0).toNumber(),
                                                ctx->argument(1).toNumber(),
                                                ctx->argument(2).toNumber(),
                                                ctx->argument(3).toNumber(),
                                                qscriptvalue_cast<QPen>(ctx->argument(4)),
                                                qscriptvalue_cast<QBrush>(ctx->argument(5))));
    } else {
        return eng->toScriptValue(self->addRect(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                                                qscriptvalue_cast<QPen>(ctx->argument(1)),
                                                qscriptvalue_cast<QBrush>(ctx->argument(2))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue addSimpleText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, addSimpleText);
    return eng->toScriptValue(self->addSimpleText(ctx->argument(0).toString(),
                                                  qscriptvalue_cast<QFont>(ctx->argument(1))));
}

/////////////////////////////////////////////////////////////

static QScriptValue addText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, addText);
    return eng->toScriptValue(self->addText(ctx->argument(0).toString(),
                                            qscriptvalue_cast<QFont>(ctx->argument(1))));
}

/////////////////////////////////////////////////////////////

static QScriptValue backgroundBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, backgroundBrush);
    return eng->toScriptValue(self->backgroundBrush());
}

/////////////////////////////////////////////////////////////

static QScriptValue bspTreeDepth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, bspTreeDepth);
    return QScriptValue(eng, self->bspTreeDepth());
}

/////////////////////////////////////////////////////////////

static QScriptValue clearFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, clearFocus);
    self->clearFocus();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue clearSelection(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, clearSelection);
    self->clearSelection();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue collidingItems(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, collidingItems);
    QGraphicsItem *item = qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0));
    if (ctx->argument(1).isUndefined()) {
        return eng->toScriptValue(self->collidingItems(item));
    } else {
        return eng->toScriptValue(self->collidingItems(item, static_cast<Qt::ItemSelectionMode>(ctx->argument(1).toInt32())));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue createItemGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, createItemGroup);
    return eng->toScriptValue(self->createItemGroup(qscriptvalue_cast<QList<QGraphicsItem*> >(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue destroyItemGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, destroyItemGroup);
    self->destroyItemGroup(qscriptvalue_cast<QGraphicsItemGroup*>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue focusItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, focusItem);
    return eng->toScriptValue(self->focusItem());
}

/////////////////////////////////////////////////////////////

static QScriptValue foregroundBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, foregroundBrush);
    return eng->toScriptValue(self->foregroundBrush());
}

/////////////////////////////////////////////////////////////

static QScriptValue hasFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, hasFocus);
    return QScriptValue(eng, self->hasFocus());
}

/////////////////////////////////////////////////////////////

static QScriptValue height(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, height);
    return QScriptValue(eng, self->height());
}

/////////////////////////////////////////////////////////////

static QScriptValue inputMethodQuery(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, inputMethodQuery);
    return eng->newVariant(self->inputMethodQuery(static_cast<Qt::InputMethodQuery>(ctx->argument(0).toInt32())));
}

/////////////////////////////////////////////////////////////

static QScriptValue invalidate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, invalidate);
    if (ctx->argumentCount() > 4) {
        self->invalidate(ctx->argument(0).toNumber(),
                         ctx->argument(1).toNumber(),
                         ctx->argument(2).toNumber(),
                         ctx->argument(3).toNumber(),
                         static_cast<QGraphicsScene::SceneLayers>(ctx->argument(4).toInt32()));
    } else {
        self->invalidate(ctx->argument(0).toNumber(),
                         ctx->argument(1).toNumber(),
                         ctx->argument(2).toNumber(),
                         ctx->argument(3).toNumber());
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue itemAt(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, itemAt);
    if (ctx->argumentCount() > 1) {
        return eng->toScriptValue(self->itemAt(ctx->argument(0).toNumber(),
                                               ctx->argument(1).toNumber()));
    } else {
        return eng->toScriptValue(self->itemAt(qscriptvalue_cast<QPointF>(ctx->argument(0))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue itemIndexMethod(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, itemIndexMethod);
    return QScriptValue(eng, static_cast<int>(self->itemIndexMethod()));
}

/////////////////////////////////////////////////////////////

static QScriptValue items(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, items);
    // ### overloads
    return eng->toScriptValue(self->items());
}

/////////////////////////////////////////////////////////////

static QScriptValue itemsBoundingRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, itemsBoundingRect);
    return eng->toScriptValue(self->itemsBoundingRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue mouseGrabberItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, mouseGrabberItem);
    return eng->toScriptValue(self->mouseGrabberItem());
}

/////////////////////////////////////////////////////////////

static QScriptValue removeItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, removeItem);
    QScriptValue arg = ctx->argument(0);
    QGraphicsItem *item = qscriptvalue_cast<QGraphicsItem*>(arg);
    self->removeItem(item);
    if (item)
        QScript::maybeTakeOwnership(arg);
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue render(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, render);
    // ### AspectRadioMode
    self->render(qscriptvalue_cast<QPainter*>(ctx->argument(0)),
                 qscriptvalue_cast<QRectF>(ctx->argument(1)),
                 qscriptvalue_cast<QRectF>(ctx->argument(2)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue sceneRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, sceneRect);
    return eng->toScriptValue(self->sceneRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue selectedItems(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, selectedItems);
    return eng->toScriptValue(self->selectedItems());
}

/////////////////////////////////////////////////////////////

static QScriptValue selectionArea(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, selectionArea);
    return eng->toScriptValue(self->selectionArea());
}

/////////////////////////////////////////////////////////////

static QScriptValue setBackgroundBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, setBackgroundBrush);
    self->setBackgroundBrush(qscriptvalue_cast<QBrush>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setBspTreeDepth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, setBspTreeDepth);
    self->setBspTreeDepth(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, setFocus);
    if (ctx->argument(0).isUndefined())
        self->setFocus();
    else
        self->setFocus(static_cast<Qt::FocusReason>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFocusItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, setFocusItem);
    QGraphicsItem *item = qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0));
    if (ctx->argument(1).isUndefined())
        self->setFocusItem(item);
    else
        self->setFocusItem(item, static_cast<Qt::FocusReason>(ctx->argument(1).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setForegroundBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, setForegroundBrush);
    self->setForegroundBrush(qscriptvalue_cast<QBrush>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setItemIndexMethod(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, setItemIndexMethod);
    self->setItemIndexMethod(static_cast<QGraphicsScene::ItemIndexMethod>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setSceneRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, setSceneRect);
    if (ctx->argumentCount() > 1) {
        self->setSceneRect(ctx->argument(0).toNumber(),
                           ctx->argument(1).toNumber(),
                           ctx->argument(2).toNumber(),
                           ctx->argument(3).toNumber());
    } else {
        self->setSceneRect(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setSelectionArea(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, setSelectionArea);
    if (ctx->argument(1).isUndefined()) {
        self->setSelectionArea(qscriptvalue_cast<QPainterPath>(ctx->argument(0)));
    } else {
        self->setSelectionArea(qscriptvalue_cast<QPainterPath>(ctx->argument(0)),
                               static_cast<Qt::ItemSelectionMode>(ctx->argument(1).toInt32()));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue update(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, update);
    self->update(ctx->argument(0).toNumber(),
                 ctx->argument(1).toNumber(),
                 ctx->argument(2).toNumber(),
                 ctx->argument(3).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue views(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, views);
    return eng->toScriptValue(self->views());
}

/////////////////////////////////////////////////////////////

static QScriptValue width(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, width);
    return QScriptValue(eng, self->width());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsScene, toString);
    return QScriptValue(eng, "QGraphicsScene");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGraphicsSceneClass(QScriptEngine *eng)
{
    QScriptValue proto = eng->newQObject(new QGraphicsScene(), QScriptEngine::AutoOwnership);
    ADD_PROTO_FUNCTION(proto, addEllipse);
    ADD_PROTO_FUNCTION(proto, addItem);
    ADD_PROTO_FUNCTION(proto, addLine);
    ADD_PROTO_FUNCTION(proto, addPath);
    ADD_PROTO_FUNCTION(proto, addPixmap);
    ADD_PROTO_FUNCTION(proto, addPolygon);
    ADD_PROTO_FUNCTION(proto, addRect);
    ADD_PROTO_FUNCTION(proto, addSimpleText);
    ADD_PROTO_FUNCTION(proto, addText);
    ADD_PROTO_FUNCTION(proto, backgroundBrush);
    ADD_PROTO_FUNCTION(proto, bspTreeDepth);
    ADD_PROTO_FUNCTION(proto, clearFocus);
    ADD_PROTO_FUNCTION(proto, clearSelection);
    ADD_PROTO_FUNCTION(proto, collidingItems);
    ADD_PROTO_FUNCTION(proto, createItemGroup);
    ADD_PROTO_FUNCTION(proto, destroyItemGroup);
    ADD_PROTO_FUNCTION(proto, focusItem);
    ADD_PROTO_FUNCTION(proto, foregroundBrush);
    ADD_PROTO_FUNCTION(proto, hasFocus);
    ADD_PROTO_FUNCTION(proto, height);
    ADD_PROTO_FUNCTION(proto, inputMethodQuery);
    ADD_PROTO_FUNCTION(proto, invalidate);
    ADD_PROTO_FUNCTION(proto, itemAt);
    ADD_PROTO_FUNCTION(proto, itemIndexMethod);
    ADD_PROTO_FUNCTION(proto, items);
    ADD_PROTO_FUNCTION(proto, itemsBoundingRect);
    ADD_PROTO_FUNCTION(proto, mouseGrabberItem);
    ADD_PROTO_FUNCTION(proto, removeItem);
    ADD_PROTO_FUNCTION(proto, render);
    ADD_PROTO_FUNCTION(proto, sceneRect);
    ADD_PROTO_FUNCTION(proto, selectedItems);
    ADD_PROTO_FUNCTION(proto, selectionArea);
    ADD_PROTO_FUNCTION(proto, setBackgroundBrush);
    ADD_PROTO_FUNCTION(proto, setBspTreeDepth);
    ADD_PROTO_FUNCTION(proto, setFocus);
    ADD_PROTO_FUNCTION(proto, setFocusItem);
    ADD_PROTO_FUNCTION(proto, setForegroundBrush);
    ADD_PROTO_FUNCTION(proto, setItemIndexMethod);
    ADD_PROTO_FUNCTION(proto, setSceneRect);
    ADD_PROTO_FUNCTION(proto, setSelectionArea);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, update);
    ADD_PROTO_FUNCTION(proto, views);
    ADD_PROTO_FUNCTION(proto, width);

    eng->setDefaultPrototype(qMetaTypeId<QGraphicsScene*>(), proto);

    qScriptRegisterSequenceMetaType<QList<QGraphicsItem*> >(eng);
    qScriptRegisterSequenceMetaType<QList<QGraphicsView*> >(eng);

    return eng->newFunction(ctor, proto);
}
