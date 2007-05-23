#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QAbstractGraphicsShapeItem>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include "../global.h"

DECLARE_POINTER_METATYPE(QAbstractGraphicsShapeItem)

static inline QScriptValue newAbstractGraphicsShapeItem(QScriptEngine *eng, QAbstractGraphicsShapeItem *item)
{
    return QScript::wrapGVPointer(eng, item);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError("QAbstractGraphicsShapeItem cannot be instantiated");
}

static QScriptValue brush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(AbstractGraphicsShapeItem, brush);
    return eng->toScriptValue(self->brush());
}

static QScriptValue pen(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(AbstractGraphicsShapeItem, pen);
    return eng->toScriptValue(self->pen());
}

static QScriptValue setBrush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(AbstractGraphicsShapeItem, setBrush);
    self->setBrush(qscriptvalue_cast<QBrush>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue setPen(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(AbstractGraphicsShapeItem, setPen);
    self->setPen(qscriptvalue_cast<QPen>(ctx->argument(0)));
    return eng->undefinedValue();
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(AbstractGraphicsShapeItem, toString);
    return QScriptValue(eng, "QAbstractGraphicsShapeItem");
}

class PrototypeAbstractGraphicsShapeItem : public QAbstractGraphicsShapeItem
{
public:
    PrototypeAbstractGraphicsShapeItem()
    { }
    QRectF boundingRect() const
    { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
    { }
};

QScriptValue constructAbstractGraphicsShapeItemClass(QScriptEngine *eng)
{
    QScriptValue proto = newAbstractGraphicsShapeItem(eng, new PrototypeAbstractGraphicsShapeItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QGraphicsItem*>()));

    ADD_PROTO_FUNCTION(proto, brush);
    ADD_PROTO_FUNCTION(proto, pen);
    ADD_PROTO_FUNCTION(proto, setBrush);
    ADD_PROTO_FUNCTION(proto, setPen);
    ADD_PROTO_FUNCTION(proto, toString);

    QScript::registerPointerMetaType<QAbstractGraphicsShapeItem>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}
