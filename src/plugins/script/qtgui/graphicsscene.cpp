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
    DECLARE_SELF(QGraphicsScene, addEllipse);
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
    DECLARE_SELF(QGraphicsScene, addItem);
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
    DECLARE_SELF(QGraphicsScene, addLine);
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
    DECLARE_SELF(QGraphicsScene, addPath);
    return eng->toScriptValue(self->addPath(qscriptvalue_cast<QPainterPath>(ctx->argument(0)),
                                            qscriptvalue_cast<QPen>(ctx->argument(1)),
                                            qscriptvalue_cast<QBrush>(ctx->argument(2))));
                                            
}

/////////////////////////////////////////////////////////////

static QScriptValue addPixmap(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, addPixmap);
    return eng->toScriptValue(self->addPixmap(qscriptvalue_cast<QPixmap>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue addPolygon(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, addPolygon);
    return eng->toScriptValue(self->addPolygon(qscriptvalue_cast<QPolygonF>(ctx->argument(0)),
                                               qscriptvalue_cast<QPen>(ctx->argument(1)),
                                               qscriptvalue_cast<QBrush>(ctx->argument(2))));
}

/////////////////////////////////////////////////////////////

static QScriptValue addRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, addRect);
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
    DECLARE_SELF(QGraphicsScene, addSimpleText);
    return eng->toScriptValue(self->addSimpleText(ctx->argument(0).toString(),
                                                  qscriptvalue_cast<QFont>(ctx->argument(1))));
}

/////////////////////////////////////////////////////////////

static QScriptValue addText(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, addText);
    return eng->toScriptValue(self->addText(ctx->argument(0).toString(),
                                            qscriptvalue_cast<QFont>(ctx->argument(1))));
}

/////////////////////////////////////////////////////////////

static QScriptValue backgroundBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, backgroundBrush);
    return eng->toScriptValue(self->backgroundBrush());
}

/////////////////////////////////////////////////////////////

static QScriptValue bspTreeDepth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, bspTreeDepth);
    return QScriptValue(eng, self->bspTreeDepth());
}

/////////////////////////////////////////////////////////////

static QScriptValue clearFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, clearFocus);
    self->clearFocus();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue clearSelection(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, clearSelection);
    self->clearSelection();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue collidingItems(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, collidingItems);
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
    DECLARE_SELF(QGraphicsScene, createItemGroup);
    return eng->toScriptValue(self->createItemGroup(qscriptvalue_cast<QList<QGraphicsItem*> >(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue destroyItemGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, destroyItemGroup);
    self->destroyItemGroup(qscriptvalue_cast<QGraphicsItemGroup*>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue focusItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, focusItem);
    return eng->toScriptValue(self->focusItem());
}

/////////////////////////////////////////////////////////////

static QScriptValue foregroundBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, foregroundBrush);
    return eng->toScriptValue(self->foregroundBrush());
}

/////////////////////////////////////////////////////////////

static QScriptValue hasFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, hasFocus);
    return QScriptValue(eng, self->hasFocus());
}

/////////////////////////////////////////////////////////////

static QScriptValue height(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, height);
    return QScriptValue(eng, self->height());
}

/////////////////////////////////////////////////////////////

static QScriptValue inputMethodQuery(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, inputMethodQuery);
    return eng->newVariant(self->inputMethodQuery(static_cast<Qt::InputMethodQuery>(ctx->argument(0).toInt32())));
}

/////////////////////////////////////////////////////////////

static QScriptValue invalidate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, invalidate);
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
    DECLARE_SELF(QGraphicsScene, itemAt);
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
    DECLARE_SELF(QGraphicsScene, itemIndexMethod);
    return QScriptValue(eng, static_cast<int>(self->itemIndexMethod()));
}

/////////////////////////////////////////////////////////////

static QScriptValue items(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, items);
    // ### overloads
    return eng->toScriptValue(self->items());
}

/////////////////////////////////////////////////////////////

static QScriptValue itemsBoundingRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, itemsBoundingRect);
    return eng->toScriptValue(self->itemsBoundingRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue mouseGrabberItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, mouseGrabberItem);
    return eng->toScriptValue(self->mouseGrabberItem());
}

/////////////////////////////////////////////////////////////

static QScriptValue removeItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, removeItem);
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
    DECLARE_SELF(QGraphicsScene, render);
    // ### AspectRadioMode
    self->render(qscriptvalue_cast<QPainter*>(ctx->argument(0)),
                 qscriptvalue_cast<QRectF>(ctx->argument(1)),
                 qscriptvalue_cast<QRectF>(ctx->argument(2)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue sceneRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, sceneRect);
    return eng->toScriptValue(self->sceneRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue selectedItems(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, selectedItems);
    return eng->toScriptValue(self->selectedItems());
}

/////////////////////////////////////////////////////////////

static QScriptValue selectionArea(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, selectionArea);
    return eng->toScriptValue(self->selectionArea());
}

/////////////////////////////////////////////////////////////

static QScriptValue setBackgroundBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, setBackgroundBrush);
    self->setBackgroundBrush(qscriptvalue_cast<QBrush>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setBspTreeDepth(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, setBspTreeDepth);
    self->setBspTreeDepth(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, setFocus);
    if (ctx->argument(0).isUndefined())
        self->setFocus();
    else
        self->setFocus(static_cast<Qt::FocusReason>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFocusItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, setFocusItem);
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
    DECLARE_SELF(QGraphicsScene, setForegroundBrush);
    self->setForegroundBrush(qscriptvalue_cast<QBrush>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setItemIndexMethod(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, setItemIndexMethod);
    self->setItemIndexMethod(static_cast<QGraphicsScene::ItemIndexMethod>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setSceneRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, setSceneRect);
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
    DECLARE_SELF(QGraphicsScene, setSelectionArea);
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
    DECLARE_SELF(QGraphicsScene, update);
    self->update(ctx->argument(0).toNumber(),
                 ctx->argument(1).toNumber(),
                 ctx->argument(2).toNumber(),
                 ctx->argument(3).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue views(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, views);
    return eng->toScriptValue(self->views());
}

/////////////////////////////////////////////////////////////

static QScriptValue width(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, width);
    return QScriptValue(eng, self->width());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QGraphicsScene, toString);
    return QScriptValue(eng, "QGraphicsScene");
}

/////////////////////////////////////////////////////////////

QScriptValue constructGraphicsSceneClass(QScriptEngine *eng)
{
    QScriptValue proto = eng->newQObject(new QGraphicsScene(), QScriptEngine::AutoOwnership);
    ADD_METHOD(proto, addEllipse);
    ADD_METHOD(proto, addItem);
    ADD_METHOD(proto, addLine);
    ADD_METHOD(proto, addPath);
    ADD_METHOD(proto, addPixmap);
    ADD_METHOD(proto, addPolygon);
    ADD_METHOD(proto, addRect);
    ADD_METHOD(proto, addSimpleText);
    ADD_METHOD(proto, addText);
    ADD_METHOD(proto, backgroundBrush);
    ADD_METHOD(proto, bspTreeDepth);
    ADD_METHOD(proto, clearFocus);
    ADD_METHOD(proto, clearSelection);
    ADD_METHOD(proto, collidingItems);
    ADD_METHOD(proto, createItemGroup);
    ADD_METHOD(proto, destroyItemGroup);
    ADD_METHOD(proto, focusItem);
    ADD_METHOD(proto, foregroundBrush);
    ADD_METHOD(proto, hasFocus);
    ADD_METHOD(proto, height);
    ADD_METHOD(proto, inputMethodQuery);
    ADD_METHOD(proto, invalidate);
    ADD_METHOD(proto, itemAt);
    ADD_METHOD(proto, itemIndexMethod);
    ADD_METHOD(proto, items);
    ADD_METHOD(proto, itemsBoundingRect);
    ADD_METHOD(proto, mouseGrabberItem);
    ADD_METHOD(proto, removeItem);
    ADD_METHOD(proto, render);
    ADD_METHOD(proto, sceneRect);
    ADD_METHOD(proto, selectedItems);
    ADD_METHOD(proto, selectionArea);
    ADD_METHOD(proto, setBackgroundBrush);
    ADD_METHOD(proto, setBspTreeDepth);
    ADD_METHOD(proto, setFocus);
    ADD_METHOD(proto, setFocusItem);
    ADD_METHOD(proto, setForegroundBrush);
    ADD_METHOD(proto, setItemIndexMethod);
    ADD_METHOD(proto, setSceneRect);
    ADD_METHOD(proto, setSelectionArea);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, update);
    ADD_METHOD(proto, views);
    ADD_METHOD(proto, width);

    eng->setDefaultPrototype(qMetaTypeId<QGraphicsScene*>(), proto);

    qScriptRegisterSequenceMetaType<QList<QGraphicsItem*> >(eng);
    qScriptRegisterSequenceMetaType<QList<QGraphicsView*> >(eng);

    return eng->newFunction(ctor, proto);
}
