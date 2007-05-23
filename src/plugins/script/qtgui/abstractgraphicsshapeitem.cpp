#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QAbstractGraphicsShapeItem>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include "../global.h"

DECLARE_POINTER_METATYPE(QAbstractGraphicsShapeItem)

DECLARE_GET_SET_METHODS(QAbstractGraphicsShapeItem, QBrush, brush, setBrush)
DECLARE_GET_SET_METHODS(QAbstractGraphicsShapeItem, QPen, pen, setPen)

static inline QScriptValue newAbstractGraphicsShapeItem(QScriptEngine *eng, QAbstractGraphicsShapeItem *item)
{
    return QScript::wrapGVPointer(eng, item);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *)
{
    return ctx->throwError("QAbstractGraphicsShapeItem cannot be instantiated");
}

BEGIN_DECLARE_METHOD(QAbstractGraphicsShapeItem, toString) {
    return QScriptValue(eng, "QAbstractGraphicsShapeItem");
} END_DECLARE_METHOD

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

    ADD_GET_SET_METHODS(proto, brush, setBrush);
    ADD_GET_SET_METHODS(proto, pen, setPen);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QAbstractGraphicsShapeItem>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);

    return ctorFun;
}
