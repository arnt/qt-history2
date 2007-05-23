#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QGraphicsEllipseItem>
#include "../global.h"

DECLARE_POINTER_METATYPE(QGraphicsEllipseItem)
Q_DECLARE_METATYPE(QAbstractGraphicsShapeItem*)

DECLARE_GET_SET_METHODS(QGraphicsEllipseItem, QRectF, rect, setRect)
DECLARE_INT_GET_SET_METHODS(QGraphicsEllipseItem, spanAngle, setSpanAngle)
DECLARE_INT_GET_SET_METHODS(QGraphicsEllipseItem, startAngle, setStartAngle)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() >= 4) {
        return QScript::wrapGVPointer(eng,
            new QGraphicsEllipseItem(ctx->argument(0).toNumber(),
                                     ctx->argument(1).toNumber(),
                                     ctx->argument(2).toNumber(),
                                     ctx->argument(3).toNumber(),
                                     qscriptvalue_cast<QGraphicsItem*>(ctx->argument(4))));
    } else if (ctx->argumentCount() > 1) {
        return QScript::wrapGVPointer(eng,
            new QGraphicsEllipseItem(qscriptvalue_cast<QRectF>(ctx->argument(0)),
                                     qscriptvalue_cast<QGraphicsItem*>(ctx->argument(1))));
    } else {
        return QScript::wrapGVPointer(eng,
            new QGraphicsEllipseItem(qscriptvalue_cast<QGraphicsItem*>(ctx->argument(0))));
    }
}

BEGIN_DECLARE_METHOD(QGraphicsEllipseItem, toString) {
    return QScriptValue(eng, "QGraphicsEllipseItem");
} END_DECLARE_METHOD

QScriptValue constructGraphicsEllipseItemClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapGVPointer(eng, new QGraphicsEllipseItem());
    proto.setPrototype(eng->defaultPrototype(qMetaTypeId<QAbstractGraphicsShapeItem*>()));

    ADD_GET_SET_METHODS(proto, rect, setRect);
    ADD_GET_SET_METHODS(proto, spanAngle, setSpanAngle);
    ADD_GET_SET_METHODS(proto, startAngle, setStartAngle);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsEllipseItem>(eng, proto);

    return eng->newFunction(ctor, proto);
}
