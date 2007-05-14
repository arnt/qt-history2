#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QCursor>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsScene>
#include "../global.h"

Q_DECLARE_METATYPE(QScript::Wrapper<QGraphicsItem*>::pointer_type)
Q_DECLARE_METATYPE(QList<QGraphicsItem*>)
Q_DECLARE_METATYPE(QPainterPath)
#ifndef QT_NO_CURSOR
Q_DECLARE_METATYPE(QCursor)
#endif
Q_DECLARE_METATYPE(QGraphicsItemGroup*)
Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(QStyleOptionGraphicsItem*)

Q_DECLARE_METATYPE(QGraphicsPathItem*)
Q_DECLARE_METATYPE(QGraphicsRectItem*)
Q_DECLARE_METATYPE(QGraphicsEllipseItem*)
Q_DECLARE_METATYPE(QGraphicsPolygonItem*)
Q_DECLARE_METATYPE(QGraphicsLineItem*)
Q_DECLARE_METATYPE(QGraphicsPixmapItem*)
Q_DECLARE_METATYPE(QGraphicsTextItem*)
Q_DECLARE_METATYPE(QGraphicsSimpleTextItem*)

/////////////////////////////////////////////////////////////

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError("QGraphicsItem cannot be instantiated");
}

/////////////////////////////////////////////////////////////

static QScriptValue acceptDrops(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, acceptDrops);
    return QScriptValue(eng, self->acceptDrops());
}

/////////////////////////////////////////////////////////////

static QScriptValue acceptedMouseButtons(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, acceptedMouseButtons);
    return QScriptValue(eng, static_cast<int>(self->acceptedMouseButtons()));
}

/////////////////////////////////////////////////////////////

static QScriptValue acceptsHoverEvents(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, acceptsHoverEvents);
    return QScriptValue(eng, self->acceptsHoverEvents());
}

/////////////////////////////////////////////////////////////

static QScriptValue advance(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, advance);
    self->advance(ctx->argument(0).toInt32());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue boundingRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, boundingRect);
    return eng->toScriptValue(self->boundingRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue children(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, children);
    return eng->toScriptValue(self->children());
}

/////////////////////////////////////////////////////////////

static QScriptValue childrenBoundingRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, childrenBoundingRect);
    return eng->toScriptValue(self->childrenBoundingRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue clearFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, clearFocus);
    self->clearFocus();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue collidesWithItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, collidesWithItem);
    QGraphicsItem *other = qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0));
    if (!other) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QGraphicsItem.prototype.collidesWithItem: argument is not a GraphicsItem");
    }
    if (ctx->argument(1).isUndefined())
        return QScriptValue(eng, self->collidesWithItem(other));
    else
        return QScriptValue(eng, self->collidesWithItem(other, static_cast<Qt::ItemSelectionMode>(ctx->argument(1).toInt32())));
}

/////////////////////////////////////////////////////////////

static QScriptValue collidesWithPath(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, collidesWithPath);
    QPainterPath path = qscriptvalue_cast<QPainterPath>(ctx->argument(0));
    if (ctx->argument(1).isUndefined())
        return QScriptValue(eng, self->collidesWithPath(path));
    else
        return QScriptValue(eng, self->collidesWithPath(path, static_cast<Qt::ItemSelectionMode>(ctx->argument(1).toInt32())));
}

/////////////////////////////////////////////////////////////

static QScriptValue collidingItems(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, collidingItems);
    if (ctx->argument(0).isUndefined())
        return eng->toScriptValue(self->collidingItems());
    else
        return eng->toScriptValue(self->collidingItems(static_cast<Qt::ItemSelectionMode>(ctx->argument(0).toInt32())));
}

/////////////////////////////////////////////////////////////

static QScriptValue contains(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, contains);
    return QScriptValue(eng, self->contains(qscriptvalue_cast<QPointF>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue cursor(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_CURSOR
    DECLARE_SELF(GraphicsItem, cursor);
    return eng->toScriptValue(self->cursor());
#else
    Q_UNUSED(ctx);
    Q_UNUSED(eng);
    return eng->undefinedValue();
#endif
}

/////////////////////////////////////////////////////////////

static QScriptValue data(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, data);
    return eng->newVariant(self->data(ctx->argument(0).toInt32()));
}

/////////////////////////////////////////////////////////////

static QScriptValue ensureVisible(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItem, ensureVisible);
    return ctx->throwError("QGraphicsItem.prototype.ensureVisible is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue flags(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, flags);
    return QScriptValue(eng, static_cast<int>(self->flags()));
}

/////////////////////////////////////////////////////////////

static QScriptValue group(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, group);
    return eng->toScriptValue(self->group());
}

/////////////////////////////////////////////////////////////

static QScriptValue handlesChildEvents(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, handlesChildEvents);
    return QScriptValue(eng, self->handlesChildEvents());
}

/////////////////////////////////////////////////////////////

static QScriptValue hasCursor(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_CURSOR
    DECLARE_SELF(GraphicsItem, hasCursor);
    return QScriptValue(eng, self->hasCursor());
#else
    Q_UNUSED(ctx);
    Q_UNUSED(eng);
    return eng->undefinedValue();
#endif
}

/////////////////////////////////////////////////////////////

static QScriptValue hasFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, hasFocus);
    return QScriptValue(eng, self->hasFocus());
}

/////////////////////////////////////////////////////////////

static QScriptValue hide(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, hide);
    self->hide();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue installSceneEventFilter(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, installSceneEventFilter);
    self->installSceneEventFilter(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue isAncestorOf(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, isAncestorOf);
    return QScriptValue(eng, self->isAncestorOf(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue isEnabled(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, isEnabled);
    return QScriptValue(eng, self->isEnabled());
}

/////////////////////////////////////////////////////////////

static QScriptValue isObscured(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, isObscured);
    if (ctx->argumentCount() > 1) {
        return QScriptValue(eng, self->isObscured(ctx->argument(0).toInt32(),
                                                  ctx->argument(1).toInt32(),
                                                  ctx->argument(2).toInt32(),
                                                  ctx->argument(3).toInt32()));
    } else {
        return QScriptValue(eng, self->isObscured(qscriptvalue_cast<QRectF>(ctx->argument(0))));
    }
}

/////////////////////////////////////////////////////////////

static QScriptValue isObscuredBy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, isObscuredBy);
    return QScriptValue(eng, self->isObscuredBy(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
}

/////////////////////////////////////////////////////////////

static QScriptValue isSelected(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, isSelected);
    return QScriptValue(eng, self->isSelected());
}

/////////////////////////////////////////////////////////////

static QScriptValue isVisible(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, isVisible);
    return QScriptValue(eng, self->isVisible());
}

/////////////////////////////////////////////////////////////

static QScriptValue mapFromItem(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItem, mapFromItem);
    return ctx->throwError("QGraphicsItem.prototype.mapFromItem is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue mapFromParent(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItem, mapFromParent);
    return ctx->throwError("QGraphicsItem.prototype.mapFromParent is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue mapFromScene(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItem, mapFromScene);
    return ctx->throwError("QGraphicsItem.prototype.mapFromScene is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue mapToItem(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItem, mapToItem);
    return ctx->throwError("QGraphicsItem.prototype.mapToItem is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue mapToParent(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItem, mapToParent);
    return ctx->throwError("QGraphicsItem.prototype.mapToParent is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue mapToScene(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(GraphicsItem, mapToScene);
    return ctx->throwError("QGraphicsItem.prototype.mapToScene is not implemented");
}

/////////////////////////////////////////////////////////////

static QScriptValue moveBy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, moveBy);
    self->moveBy(ctx->argument(0).toNumber(), ctx->argument(1).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue opaqueArea(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, opaqueArea);
    return eng->toScriptValue(self->opaqueArea());
}

/////////////////////////////////////////////////////////////

static QScriptValue paint(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, paint);
    self->paint(qscriptvalue_cast<QPainter*>(ctx->argument(0)),
                qscriptvalue_cast<QStyleOptionGraphicsItem*>(ctx->argument(1)),
                qscriptvalue_cast<QWidget*>(ctx->argument(2)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue parentItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, parentItem);
    QGraphicsItem *parent = self->parentItem();
    if (!parent)
        return eng->nullValue();
    QScriptValue ret = eng->toScriptValue(parent);
    QScriptValue proto;
    switch (parent->type()) {
    case 2:
        proto = eng->defaultPrototype(qMetaTypeId<QGraphicsPathItem*>());
        break;
    case 3:
        proto = eng->defaultPrototype(qMetaTypeId<QGraphicsRectItem*>());
        break;
    case 4:
        proto = eng->defaultPrototype(qMetaTypeId<QGraphicsEllipseItem*>());
        break;
    case 5:
        proto = eng->defaultPrototype(qMetaTypeId<QGraphicsPolygonItem*>());
        break;
    case 6:
        proto = eng->defaultPrototype(qMetaTypeId<QGraphicsLineItem*>());
        break;
    case 7:
        proto = eng->defaultPrototype(qMetaTypeId<QGraphicsPixmapItem*>());
        break;
    case 8:
        proto = eng->defaultPrototype(qMetaTypeId<QGraphicsTextItem*>());
        break;
    case 9:
        proto = eng->defaultPrototype(qMetaTypeId<QGraphicsSimpleTextItem*>());
        break;
    case 10:
        proto = eng->defaultPrototype(qMetaTypeId<QGraphicsItemGroup*>());
        break;
    }
    if (proto.isValid())
        ret.setPrototype(proto);
    return ret;
}

/////////////////////////////////////////////////////////////

static QScriptValue pos(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, pos);
    return eng->toScriptValue(self->pos());
}

/////////////////////////////////////////////////////////////

static QScriptValue removeSceneEventFilter(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, removeSceneEventFilter);
    self->removeSceneEventFilter(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue resetTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, resetTransform);
    self->resetTransform();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue rotate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, rotate);
    self->rotate(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue scale(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, scale);
    self->scale(ctx->argument(0).toNumber(), ctx->argument(1).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue scene(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, scene);
    return eng->newQObject(self->scene());
}

/////////////////////////////////////////////////////////////

static QScriptValue sceneBoundingRect(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, sceneBoundingRect);
    return eng->toScriptValue(self->sceneBoundingRect());
}

/////////////////////////////////////////////////////////////

static QScriptValue scenePos(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, scenePos);
    return eng->toScriptValue(self->scenePos());
}

/////////////////////////////////////////////////////////////

static QScriptValue sceneTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, sceneTransform);
    return eng->toScriptValue(self->sceneTransform());
}

/////////////////////////////////////////////////////////////

static QScriptValue setAcceptDrops(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setAcceptDrops);
    self->setAcceptDrops(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setAcceptedMouseButtons(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setAcceptedMouseButtons);
    self->setAcceptedMouseButtons(static_cast<Qt::MouseButtons>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setAcceptsHoverEvents(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setAcceptsHoverEvents);
    self->setAcceptsHoverEvents(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setCursor(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_CURSOR
    DECLARE_SELF(GraphicsItem, setCursor);
    self->setCursor(qscriptvalue_cast<QCursor>(ctx->argument(0)));
    return eng->undefinedValue();
#else
    Q_UNUSED(ctx);
    Q_UNUSED(eng);
    return eng->undefinedValue();
#endif
}

/////////////////////////////////////////////////////////////

static QScriptValue setData(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setData);
    self->setData(ctx->argument(0).toInt32(), ctx->argument(1).toVariant());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setEnabled(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setEnabled);
    self->setEnabled(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFlag(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setFlag);
    QGraphicsItem::GraphicsItemFlag flag = static_cast<QGraphicsItem::GraphicsItemFlag>(ctx->argument(0).toInt32());
    if (ctx->argument(1).isUndefined())
        self->setFlag(flag);
    else
        self->setFlag(flag, ctx->argument(1).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFlags(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setFlags);
    self->setFlags(static_cast<QGraphicsItem::GraphicsItemFlags>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setFocus(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setFocus);
    if (ctx->argument(0).isUndefined())
        self->setFocus();
    else
        self->setFocus(static_cast<Qt::FocusReason>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setGroup(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setGroup);
    self->setGroup(qscriptvalue_cast<QGraphicsItemGroup*>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setHandlesChildEvents(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setHandlesChildEvents);
    self->setHandlesChildEvents(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setParentItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setParentItem);
    QScriptValue arg = ctx->argument(0);
    QGraphicsItem *item = qscriptvalue_cast<QGraphicsItem*>(arg);
    self->setParentItem(item);
    if (item)
        QScript::maybeReleaseOwnership(ctx->thisObject());
    else if (!self->scene())
        QScript::maybeTakeOwnership(ctx->thisObject());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setPos(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setPos);
    if (ctx->argumentCount() > 1)
        self->setPos(ctx->argument(0).toNumber(), ctx->argument(1).toNumber());
    else
        self->setPos(qscriptvalue_cast<QPointF>(ctx->argument(0)));
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setSelected(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setSelected);
    self->setSelected(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setToolTip(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_TOOLTIP
    DECLARE_SELF(GraphicsItem, setToolTip);
    self->setToolTip(ctx->argument(0).toString());
    return eng->undefinedValue();
#else
    Q_UNUSED(ctx);
    Q_UNUSED(eng);
    return eng->undefinedValue();
#endif
}

/////////////////////////////////////////////////////////////

static QScriptValue setTransform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setTransform);
    self->setTransform(qscriptvalue_cast<QTransform>(ctx->argument(0)),
                       ctx->argument(1).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setVisible(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setVisible);
    self->setVisible(ctx->argument(0).toBoolean());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue setZValue(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, setZValue);
    self->setZValue(ctx->argument(0).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue shape(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, shape);
    return eng->toScriptValue(self->shape());
}

/////////////////////////////////////////////////////////////

static QScriptValue shear(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, shear);
    self->shear(ctx->argument(0).toNumber(), ctx->argument(1).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue show(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, show);
    self->show();
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue toolTip(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_TOOLTIP
    DECLARE_SELF(GraphicsItem, toolTip);
    return QScriptValue(eng, self->toolTip());
#else
    Q_UNUSED(ctx);
    Q_UNUSED(eng);
    return eng->undefinedValue();
#endif
}

/////////////////////////////////////////////////////////////

static QScriptValue topLevelItem(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, topLevelItem);
    return eng->toScriptValue(self->topLevelItem());
}

/////////////////////////////////////////////////////////////

static QScriptValue transform(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, transform);
    return eng->toScriptValue(self->transform());
}

/////////////////////////////////////////////////////////////

static QScriptValue translate(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, translate);
    self->translate(ctx->argument(0).toNumber(), ctx->argument(1).toNumber());
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue type(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, type);
    return QScriptValue(eng, self->type());
}

/////////////////////////////////////////////////////////////

static QScriptValue unsetCursor(QScriptContext *ctx, QScriptEngine *eng)
{
#ifndef QT_NO_CURSOR
    DECLARE_SELF(GraphicsItem, unsetCursor);
    self->unsetCursor();
    return eng->undefinedValue();
#else
    Q_UNUSED(ctx);
    Q_UNUSED(eng);
    return eng->undefinedValue();
#endif
}

/////////////////////////////////////////////////////////////

static QScriptValue update(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, update);
    if (ctx->argumentCount() > 1) {
        self->update(ctx->argument(0).toNumber(),
                     ctx->argument(1).toNumber(),
                     ctx->argument(2).toNumber(),
                     ctx->argument(3).toNumber());
    } else {
        self->update(qscriptvalue_cast<QRectF>(ctx->argument(0)));
    }
    return eng->undefinedValue();
}

/////////////////////////////////////////////////////////////

static QScriptValue x(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, x);
    return QScriptValue(eng, self->x());
}

/////////////////////////////////////////////////////////////

static QScriptValue y(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, y);
    return QScriptValue(eng, self->y());
}

/////////////////////////////////////////////////////////////

static QScriptValue zValue(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, zValue);
    return QScriptValue(eng, self->zValue());
}

/////////////////////////////////////////////////////////////

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(GraphicsItem, toString);
    return QScriptValue(eng, "QGraphicsItem");
}

/////////////////////////////////////////////////////////////

class PrototypeGraphicsItem : public QGraphicsItem
{
public:
    PrototypeGraphicsItem()
    { }
    QRectF boundingRect() const
    { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
    { }
};

QScriptValue constructGraphicsItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::construct<QGraphicsItem>(eng, new PrototypeGraphicsItem());
    ADD_PROTO_FUNCTION(proto, acceptDrops);
    ADD_PROTO_FUNCTION(proto, acceptedMouseButtons);
    ADD_PROTO_FUNCTION(proto, acceptsHoverEvents);
    ADD_PROTO_FUNCTION(proto, advance);
    ADD_PROTO_FUNCTION(proto, boundingRect);
    ADD_PROTO_FUNCTION(proto, children);
    ADD_PROTO_FUNCTION(proto, childrenBoundingRect);
    ADD_PROTO_FUNCTION(proto, clearFocus);
    ADD_PROTO_FUNCTION(proto, collidesWithItem);
    ADD_PROTO_FUNCTION(proto, collidesWithPath);
    ADD_PROTO_FUNCTION(proto, collidingItems);
    ADD_PROTO_FUNCTION(proto, contains);
    ADD_PROTO_FUNCTION(proto, cursor);
    ADD_PROTO_FUNCTION(proto, data);
    ADD_PROTO_FUNCTION(proto, ensureVisible);
    ADD_PROTO_FUNCTION(proto, flags);
    ADD_PROTO_FUNCTION(proto, group);
    ADD_PROTO_FUNCTION(proto, handlesChildEvents);
    ADD_PROTO_FUNCTION(proto, hasCursor);
    ADD_PROTO_FUNCTION(proto, hasFocus);
    ADD_PROTO_FUNCTION(proto, hide);
    ADD_PROTO_FUNCTION(proto, installSceneEventFilter);
    ADD_PROTO_FUNCTION(proto, isAncestorOf);
    ADD_PROTO_FUNCTION(proto, isEnabled);
    ADD_PROTO_FUNCTION(proto, isObscured);
    ADD_PROTO_FUNCTION(proto, isObscuredBy);
    ADD_PROTO_FUNCTION(proto, isSelected);
    ADD_PROTO_FUNCTION(proto, isVisible);
    ADD_PROTO_FUNCTION(proto, mapFromItem);
    ADD_PROTO_FUNCTION(proto, mapFromParent);
    ADD_PROTO_FUNCTION(proto, mapFromScene);
    ADD_PROTO_FUNCTION(proto, mapToItem);
    ADD_PROTO_FUNCTION(proto, mapToParent);
    ADD_PROTO_FUNCTION(proto, mapToScene);
    ADD_PROTO_FUNCTION(proto, moveBy);
    ADD_PROTO_FUNCTION(proto, opaqueArea);
    ADD_PROTO_FUNCTION(proto, paint);
    ADD_PROTO_FUNCTION(proto, parentItem);
    ADD_PROTO_FUNCTION(proto, pos);
    ADD_PROTO_FUNCTION(proto, removeSceneEventFilter);
    ADD_PROTO_FUNCTION(proto, resetTransform);
    ADD_PROTO_FUNCTION(proto, rotate);
    ADD_PROTO_FUNCTION(proto, scale);
    ADD_PROTO_FUNCTION(proto, scene);
    ADD_PROTO_FUNCTION(proto, sceneBoundingRect);
    ADD_PROTO_FUNCTION(proto, scenePos);
    ADD_PROTO_FUNCTION(proto, sceneTransform);
    ADD_PROTO_FUNCTION(proto, setAcceptDrops);
    ADD_PROTO_FUNCTION(proto, setAcceptedMouseButtons);
    ADD_PROTO_FUNCTION(proto, setAcceptsHoverEvents);
    ADD_PROTO_FUNCTION(proto, setCursor);
    ADD_PROTO_FUNCTION(proto, setData);
    ADD_PROTO_FUNCTION(proto, setEnabled);
    ADD_PROTO_FUNCTION(proto, setFlag);
    ADD_PROTO_FUNCTION(proto, setFlags);
    ADD_PROTO_FUNCTION(proto, setFocus);
    ADD_PROTO_FUNCTION(proto, setGroup);
    ADD_PROTO_FUNCTION(proto, setHandlesChildEvents);
    ADD_PROTO_FUNCTION(proto, setParentItem);
    ADD_PROTO_FUNCTION(proto, setPos);
    ADD_PROTO_FUNCTION(proto, setSelected);
    ADD_PROTO_FUNCTION(proto, setToolTip);
    ADD_PROTO_FUNCTION(proto, setTransform);
    ADD_PROTO_FUNCTION(proto, setVisible);
    ADD_PROTO_FUNCTION(proto, setZValue);
    ADD_PROTO_FUNCTION(proto, shape);
    ADD_PROTO_FUNCTION(proto, shear);
    ADD_PROTO_FUNCTION(proto, show);
    ADD_PROTO_FUNCTION(proto, toolTip);
    ADD_PROTO_FUNCTION(proto, topLevelItem);
    ADD_PROTO_FUNCTION(proto, toString);
    ADD_PROTO_FUNCTION(proto, transform);
    ADD_PROTO_FUNCTION(proto, translate);
    ADD_PROTO_FUNCTION(proto, type);
    ADD_PROTO_FUNCTION(proto, unsetCursor);
    ADD_PROTO_FUNCTION(proto, update);
    ADD_PROTO_FUNCTION(proto, x);
    ADD_PROTO_FUNCTION(proto, y);
    ADD_PROTO_FUNCTION(proto, zValue);

    QScript::registerMetaTypeWrapper<QScript::Wrapper<QGraphicsItem*> >(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    ADD_ENUM_VALUE(ctorFun, QGraphicsItem, ItemIsMovable);
    ADD_ENUM_VALUE(ctorFun, QGraphicsItem, ItemIsSelectable);
    ADD_ENUM_VALUE(ctorFun, QGraphicsItem, ItemIsFocusable);
    ADD_ENUM_VALUE(ctorFun, QGraphicsItem, ItemClipsToShape);
    ADD_ENUM_VALUE(ctorFun, QGraphicsItem, ItemClipsChildrenToShape);
    ADD_ENUM_VALUE(ctorFun, QGraphicsItem, ItemIgnoresTransformations);

    return ctorFun;
}
